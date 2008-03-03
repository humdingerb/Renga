//////////////////////////////////////////////////
// Blabber [JabberSpeak.cpp]
//////////////////////////////////////////////////

#ifndef JABBER_SPEAK_H
	#include "JabberSpeak.h"
#endif

#ifndef __CSTDIO__
	#include <cstdio>
#endif

#ifndef _ROSTER_H
	#include "app/Roster.h"
#endif

#ifndef _UNI_STD_H
	#include "unistd.h"
#endif

#ifndef AGENT_LIST_H
	#include "AgentList.h"
#endif

#ifndef GENERIC_FUNCTIONS_H
	#include "GenericFunctions.h"
#endif

#ifndef JROSTER_H
	#include "JRoster.h"
#endif

#ifndef MODAL_ALERT_FACTORY_H
	#include "ModalAlertFactory.h"
#endif

#ifndef MESSAGE_REPEATER_H
	#include "MessageRepeater.h"
#endif

#ifndef MESSAGES_H
	#include "Messages.h"
#endif

#ifndef TALK_MANAGER_H
	#include "TalkManager.h"
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

#ifndef XML_ENTITY_H
	#include "XMLEntity.h"
#endif

JabberSpeak *JabberSpeak::_instance = NULL;

//////////////////////////////////////////////////
// CREATORS
//////////////////////////////////////////////////

JabberSpeak *JabberSpeak::Instance() {
	if (_instance == NULL) {
		_instance = new JabberSpeak();
	}
	
	return _instance;
}

JabberSpeak::JabberSpeak()
	: PortTalker(), BLooper(), XMLReader() {
	_registering_new_account = false;
	
	// base network
	OnDisconnect(JabberSpeak::StaticCalledOnDisconnect, this);

	// create semaphore
	_xml_reader_lock = create_sem(1, "xml reader semaphore");

	// add self to message family
	MessageRepeater::Instance()->AddTarget(this);

	// direct PortTalker functionality at self
	SetTargetLooper(MessageRepeater::Instance());

	// grab a handle to the settings now for convenience later
	_blabber_settings = BlabberSettings::Instance();
}

JabberSpeak::~JabberSpeak() {
	// remove self from message family
	MessageRepeater::Instance()->RemoveTarget(this);

	// destroy semaphore
	delete_sem(_xml_reader_lock);

	_instance = NULL;
}

void JabberSpeak::Reset(bool leave_network_alone) {
	if (!_reconnecting) {
		BlabberMainWindow::Instance()->Lock();
		BlabberMainWindow::Instance()->ShowLogin();
		BlabberMainWindow::Instance()->Unlock();
	}
	
	// reset networking
	if (!leave_network_alone) {
		Disconnect();
	}
	
	// reset XMLReader
	XMLReader::Reset();

	if (!_reconnecting) {
		TalkManager::Instance()->Reset();
	}
	
	JRoster::Instance()->Lock();
	JRoster::Instance()->RemoveAllUsers();
	JRoster::Instance()->Unlock();

	JRoster::Instance()->RefreshRoster();
	
	// reset agent list
	AgentList::Instance()->RemoveAllAgents();

	OnDisconnect(JabberSpeak::StaticCalledOnDisconnect, this);
}

void JabberSpeak::JabberSpeakReset() {
	_curr_realname           = "";
	_curr_login              = "";
	_password                = "";
	_am_logged_in            = false;
	_registering_new_account = false;
	_reconnecting            = false;
	_got_some_agent_info     = false;
	_got_some_roster_info    = false;

	_iq_map.clear();
}

void JabberSpeak::StaticCalledOnDisconnect(void *obj) {
	// call member version
	((JabberSpeak *)obj)->CalledOnDisconnect();
}

void JabberSpeak::CalledOnDisconnect() {
	Disconnect();
	
	// reset XMLReader
	XMLReader::Reset();

	if (_am_logged_in) {
		// automatic reconnection
		_reconnecting = true;
	
		MessageRepeater::Instance()->PostMessage(JAB_RECONNECTING);

		_got_some_agent_info     = false;
		_got_some_roster_info    = false;
		_am_logged_in            = false;

		// reset networking
		Reset(true);

		SendConnect();
	}
}

//////////////////////////////////////////////////
// STANDARD METHODS
//////////////////////////////////////////////////

void JabberSpeak::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case PortTalker::DATA: {
			const char *buffer = msg->FindString("data");
			int len            = msg->FindInt32("length");
			
			LockXMLReader();
			FeedData(buffer, len);
			UnlockXMLReader();

			break;
		}
	}
}

char **JabberSpeak::CreateAttributeMemory(int num_items) {
	char **atts;
	
	atts = (char **)malloc((num_items + 2) * sizeof(char *));
	for (int i=0; i<num_items; ++i)
		atts[i] = (char *)malloc(96 * sizeof(char));
	
	atts[num_items] = NULL;
	atts[num_items+1] = NULL;
	
	return atts;
}

void JabberSpeak::DestroyAttributeMemory(char **atts, int num_items) {
	for (int i=0; i<(num_items + 2); ++i) {
		free(atts[i]);
	}
	
	free(atts);
}

string JabberSpeak::GenerateUniqueID() {
	static long counter = 0;

	// element #1: PID of process (same through application run-time)
	pid_t pid = getpid();
	
	// element #2: seconds since Jan. 1, 1970 (new value every second)
	time_t secs = time(NULL);
	
	// element #3: private counter (new value every call)
	++counter;
	
	// glue number together
	char buffer[100];
	
	sprintf(buffer, "%lu:%lu:%lu", pid, secs, counter);
	
	// return value
	return string(buffer);
}

const string JabberSpeak::CurrentRealName() const {
	return _curr_realname;
}

const string JabberSpeak::CurrentLogin() const {
	return _curr_login;
}

void JabberSpeak::LockXMLReader() {
	acquire_sem(_xml_reader_lock);
}

void JabberSpeak::UnlockXMLReader() {
	release_sem(_xml_reader_lock);
}

//////////////////////////////////////////////////
// INCOMING COMMUNICATION
//////////////////////////////////////////////////

void JabberSpeak::OnStartTag(XMLEntity *entity) {
	OnTag(entity);
}

void JabberSpeak::OnEndTag(XMLEntity *entity) {
	OnTag(entity);
}

void JabberSpeak::OnEndEntity(XMLEntity *entity) {
	OnTag(entity);
}

void JabberSpeak::OnTag(XMLEntity *entity) {
	char buffer[4096]; // general buffer space
	string iq_id;      // used for IQ tags

	static int seen_streams = 0;

	// handle connection
	if (!entity->IsCompleted() && !strcasecmp(entity->Name(), "stream:stream")) {
		if (_registering_new_account) {
			// create account
			_SendUserRegistration(UserID(_curr_login).JabberUsername(), _password, UserID(_curr_login).JabberResource());
		} else {
			// log in
			_SendAuthentication(UserID(_curr_login).JabberUsername(), _password, UserID(_curr_login).JabberResource());
		}
	}

	// handle connection error
	if (entity->IsCompleted() && !strcasecmp(entity->Name(), "stream:error")) {
		sprintf(buffer, "An error has occurred between the client and server for the following reason:\n\n%s\n\nJabber for Haiku must shut down now. :-(", entity->Data());
		ModalAlertFactory::Alert(buffer, "Drats!", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 
		
		exit(1);
	}

	// handle disconnection
	if (entity->IsCompleted() && !strcasecmp(entity->Name(), "stream:stream")) {
		++seen_streams;
		
		if (seen_streams % 2 == 1) {
			Reset();

			MessageRepeater::Instance()->PostMessage(JAB_DISCONNECTED);
		}

		return;
	}

	// handle presence
	if (entity->IsCompleted() && !strcasecmp(entity->Name(), "presence")) {
		_ProcessPresence(entity);
	}

	// handle messages
	if (entity->IsCompleted() && !strcasecmp(entity->Name(), "message")) {
		TalkManager::Instance()->ProcessMessageData(entity);
	}

	// convert non-ID'd IQs to query indexed calls
	if (entity->IsCompleted() && !strcasecmp(entity->Name(), "iq") && !entity->Attribute("id")) {
		// get access to query tag
		XMLEntity *query = entity->Child("query");
		
		if (query && query->Attribute("xmlns")) {
			string unique_id = GenerateUniqueID();
			
			if (!strcasecmp(query->Attribute("xmlns"), "jabber:iq:roster")) {
				 iq_id = unique_id;
				_iq_map[unique_id] = ROSTER;
			}
		}
	}

	// handle IQs (always closing tags)
	if (entity->IsCompleted() && !strcasecmp(entity->Name(), "iq")) {
		if (entity->Attribute("id")) {
			iq_id = entity->Attribute("id");
		}

		// handle SETS only (change)
		if (!strcasecmp(entity->Attribute("type"), "set")) {
		// handle ERRORS only (failure)
			// get the intent of the IQ message
			if (_iq_map.count(iq_id) > 0) {
				// process based on the intent
				iq_intent intent = _iq_map[iq_id];

				// for roster changes
				if (intent == ROSTER) {
					_ParseRosterList(entity);
				}

				// remove the item from the list of pending IQs
				_iq_map.erase(iq_id);
			}
		}
						
		if (!strcasecmp(entity->Attribute("type"), "error")) {
			// get the intent of the IQ message
			if (_iq_map.count(iq_id) > 0) {
				// process based on the intent
				iq_intent intent = _iq_map[iq_id];

				// for errors on registration				
				if (intent == NEW_USER) {
					sprintf(buffer, "You have been refused registration for the following reason:\n\n%s", entity->Child("error")->Data());
					ModalAlertFactory::Alert(buffer, "I'll try again", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 

					JabberSpeakReset();

					BlabberMainWindow::Instance()->Lock();
					BlabberMainWindow::Instance()->ShowLogin();
					BlabberMainWindow::Instance()->Unlock();
				}

				// for errors on login				
				if (intent == LOGIN) {
					sprintf(buffer, "Your login attempt failed due to the following reason:\n\n%s", entity->Child("error")->Data());
					ModalAlertFactory::Alert(buffer, "I'll try again", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 

					Reset();
				}

				// for errors on registration
				if (intent == REGISTER) {
					if (entity->Attribute("from") && AgentList::Instance()->GetAgentByID(entity->Attribute("from"))) {
						const char *agent_name = AgentList::Instance()->GetAgentByID(entity->Attribute("from"))->Name().c_str();
						sprintf(buffer, "You were refused registration information from the %s for the following reason:\n\n%s", agent_name, entity->Child("error")->Data());
					} else {
						sprintf(buffer, "You were refused registration information from an unidentifying Jabber service for the following reason:\n\n%s", entity->Child("error")->Data());
					}

					ModalAlertFactory::Alert(buffer, "Oh, well.", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 
				}

				// for errors on registration
				if (intent == SEND_REGISTER) {
					if (entity->Attribute("from") && AgentList::Instance()->GetAgentByID(entity->Attribute("from"))) {
						const char *agent_name = AgentList::Instance()->GetAgentByID(entity->Attribute("from"))->Name().c_str();
						sprintf(buffer, "Your registration attempt was refused by the %s for the following reason:\n\n%s", agent_name, entity->Child("error")->Data());
					} else {
						sprintf(buffer, "Your registration attempt was refused by an unidentifying Jabber service for the following reason:\n\n%s", entity->Child("error")->Data());
					}

					ModalAlertFactory::Alert(buffer, "Oh, well.", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 
				}

				// for errors on unregistration
				if (intent == UNREGISTER) {
					if (entity->Attribute("from") && AgentList::Instance()->GetAgentByID(entity->Attribute("from"))) {
						const char *agent_name = AgentList::Instance()->GetAgentByID(entity->Attribute("from"))->Name().c_str();
						sprintf(buffer, "You were refused unregistration information from the %s for the following reason:\n\n%s", agent_name, entity->Child("error")->Data());
					} else {
						sprintf(buffer, "You were refused unregistration information from an unidentifying Jabber service for the following reason:\n\n%s", entity->Child("error")->Data());
					}

					ModalAlertFactory::Alert(buffer, "Oh, well.", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 
				}

				// remove the item from the list of pending IQs
				_iq_map.erase(iq_id);
			}
		}

		// handle RESULT only (success)
		if (!strcasecmp(entity->Attribute("type"), "result")) {
			// get the intent of the IQ message
			if (_iq_map.count(iq_id) > 0) {
				// process based on the intent
				iq_intent intent = _iq_map[iq_id];

				// for successful registration				
				if (intent == NEW_USER) {
					// log in
					_SendAuthentication(UserID(_curr_login).JabberUsername(), _password, UserID(_curr_login).JabberResource());
				}

				// for successful login				
				if (intent == LOGIN) {
					MessageRepeater::Instance()->PostMessage(JAB_LOGGED_IN);
						_SendAgentRequest();
						_SendRosterRequest();

						_reconnecting = false;
					
					SendLastPresence();	
				}

				// for the roster list
				if (intent == ROSTER) {
					_ParseRosterList(entity);
				}

				// for the agents list
				if (intent == AGENTS) {
					_ParseAgentList(entity);
				}

				// for registering a transport
				if (intent == REGISTER) {
					_ProcessRegistration(entity);
				}

				// for completed registration
				if (intent == SEND_REGISTER) {
					if (entity->Attribute("from") && AgentList::Instance()->GetAgentByID(entity->Attribute("from"))) {
						const char *agent_name = AgentList::Instance()->GetAgentByID(entity->Attribute("from"))->Name().c_str();
						sprintf(buffer, "Your registration attempt with the %s has been accepted.", agent_name);

						AgentList::Instance()->GetAgentByID(entity->Attribute("from"))->SetRegisteredFlag(true);			
					} else {
						sprintf(buffer, "Your registration attempt with an unidentifying Jabber service has been accepted.");
					}

					ModalAlertFactory::Alert(buffer, "Yeah!", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); 
				}

				// for unregistering from a transport
				if (intent == UNREGISTER) {
					_ProcessUnregistration(entity);
				}

				// remove the item from the list of pending IQs
				_iq_map.erase(iq_id);
			}
		}

		// IQ messages can be disposed of after use
		// BUGBUG need a way of doing this, otherwise expanding memory occurs
	}
}

void JabberSpeak::_ProcessRegistration(XMLEntity *iq_register_entity) {
	XMLEntity *entity = iq_register_entity;
	Agent     *agent;

	// get the agent
	if (entity->Attribute("from")) {
		agent = AgentList::Instance()->GetAgentByID(entity->Attribute("from"));
	} else {
		return;
	}
	
	if (agent == NULL) {
		return;
	}	

	// go one level deep to query
	if (entity->Child("query")) {
		entity = entity->Child("query");
	} else {
		return;
	}
	
	// get the key
	if (entity->Child("key")) {
		// extract the key data
		string key = entity->Child("key")->Data();

		// complete registration of the transport
		_SendTransportRegistrationInformation(agent, key);
	} else {
		return;
	}
}

void JabberSpeak::_ProcessUnregistration(XMLEntity *iq_register_entity) {
	XMLEntity *entity = iq_register_entity;
	Agent     *agent;

	// get the agent
	if (entity->Attribute("from")) {
		agent = AgentList::Instance()->GetAgentByID(entity->Attribute("from"));
	} else {
		return;
	}
	
	if (agent == NULL) {
		return;
	}	

	// go one level deep to query
	if (entity->Child("query")) {
		entity = entity->Child("query");
	} else {
		return;
	}
	
	// get the key
	if (entity->Child("key")) {
		// extract the key data
		string key = entity->Child("key")->Data();

		// complete registration of the transport
		_SendTransportUnregistrationInformation(agent, key);
	} else {
		return;
	}
}

void JabberSpeak::_SendTransportRegistrationInformation(Agent *agent, string key) {
	XMLEntity   *entity_iq, *entity_query;
	char **atts_iq    = CreateAttributeMemory(6);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts_iq[0], "id");

	strcpy(atts_iq[1], GenerateUniqueID().c_str());

	strcpy(atts_iq[2], "type");
	strcpy(atts_iq[3], "set");

	strcpy(atts_iq[4], "to");
	strcpy(atts_iq[5], agent->JID().c_str());

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:register");

	// construct XML tagset
	entity_iq    = new XMLEntity("iq", (const char **)atts_iq);
	entity_query = new XMLEntity("query", (const char **)atts_query);

	entity_iq->AddChild(entity_query);
	
	entity_query->AddChild("username", NULL, agent->Username().c_str());
	entity_query->AddChild("password", NULL, agent->Password().c_str());
	entity_query->AddChild("nick", NULL, GenericFunctions::GenerateNick(agent->Username()).c_str());
	entity_query->AddChild("key", NULL, key.c_str());

	// log command
	_iq_map[atts_iq[1]] = SEND_REGISTER;
	
	// send XML command
	char *str = entity_iq->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts_iq, 6);
	DestroyAttributeMemory(atts_query, 2);
	delete entity_iq;
}

void JabberSpeak::_SendTransportUnregistrationInformation(Agent *agent, string key) {
	XMLEntity   *entity_iq, *entity_query;
	char **atts_iq    = CreateAttributeMemory(6);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts_iq[0], "id");
	strcpy(atts_iq[1], GenerateUniqueID().c_str());

	strcpy(atts_iq[2], "type");
	strcpy(atts_iq[3], "set");

	strcpy(atts_iq[4], "to");
	strcpy(atts_iq[5], agent->JID().c_str());

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:register");

	// construct XML tagset
	entity_iq    = new XMLEntity("iq", (const char **)atts_iq);
	entity_query = new XMLEntity("query", (const char **)atts_query);

	entity_iq->AddChild(entity_query);
	
	entity_query->AddChild("remove", NULL, NULL);
	entity_query->AddChild("key", NULL, key.c_str());

	// log command
	_iq_map[atts_iq[1]] = SEND_UNREGISTER;
	
	// send XML command
	char *str = entity_iq->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts_iq, 6);
	DestroyAttributeMemory(atts_query, 2);
	delete entity_iq;
}

void JabberSpeak::_ParseRosterList(XMLEntity *iq_roster_entity) {
	XMLEntity *entity = iq_roster_entity;

	// go one level deep to query
	if (entity->Child("query")) {
		entity = entity->Child("query");
	} else {
		return;
	}
	
	// iterate through child 'item' tags
	JRoster::Instance()->Lock();
	for (int i=0; i<entity->CountChildren(); ++i) {
		// handle the item child
		if (!strcasecmp(entity->Child(i)->Name(), "item")) {
			if (!entity->Child(i)->Attribute("jid")) {
				continue;
			}

			// make a user
			UserID user(entity->Child(i)->Attribute("jid"));

			// no resources supported
			if (user.JabberResource().size()) {
				continue;
			}

			// set friendly name
			if (entity->Child(i)->Attribute("name")) {
				user.SetFriendlyName(entity->Child(i)->Attribute("name"));
			}
			
			// set subscription status
			if (entity->Child(i)->Attribute("subscription")) {
				user.SetSubscriptionStatus(entity->Child(i)->Attribute("subscription"));
			}

			// set ask
			if (entity->Child(i)->Attribute("ask")) {
				user.SetAsk(entity->Child(i)->Attribute("ask"));
			}

			// obtain a handle to the user (is there a new one?)
			UserID *roster_user;
			
			if (user.IsUser()) {
				roster_user = JRoster::Instance()->FindUser(JRoster::HANDLE, user.JabberHandle());
			} else if (user.UserType() == UserID::TRANSPORT) {
				roster_user = JRoster::Instance()->FindUser(JRoster::TRANSPORT_ID, user.TransportID());
			} else {
				continue;
			}

			// if we have duplicates, settle disputes
			if (roster_user) {
				// process if it's a removal
				if (entity->Child(i)->Attribute("subscription") && !strcasecmp(entity->Child(i)->Attribute("subscription"), "remove")) {
					// remove from the list
					JRoster::Instance()->RemoveUser(roster_user);

					continue;
				}

				// update the new roster item
				*roster_user = user;
			} else {
				// create the user
				roster_user = new UserID(entity->Child(i)->Attribute("jid"));

				*roster_user = user;

				// add to the list
				JRoster::Instance()->AddRosterUser(roster_user);
			}
		}
	}

	JRoster::Instance()->Unlock();	

	// update all RosterViews
	JRoster::Instance()->RefreshRoster();
	
	_got_some_roster_info = true;
	
	if (_got_some_agent_info && _got_some_roster_info) {
		_am_logged_in = true;
		MessageRepeater::Instance()->PostMessage(JAB_GOT_SERVER_INFO);
	}
}

void JabberSpeak::_ProcessPresence(XMLEntity *entity) {
	JRoster *roster = JRoster::Instance();

	int num_matches = 0;

	// verify we have a username
	if (entity->Attribute("from")) {
		roster->Lock();

		// circumvent groupchat presences
		string room, server, user;
		
		// split it all out
		int tokens = GenericFunctions::SeparateGroupSpecifiers(entity->Attribute("from"), room, server, user);

		if (tokens == 3 && !TalkManager::Instance()->IsExistingWindowToGroup(TalkWindow::GROUP, room + '@' + server).empty()) {
			BMessage msg;
				msg.AddString("room", (room + '@' + server).c_str());
				msg.AddString("server", server.c_str());
				msg.AddString("username", user.c_str());

			if (!entity->Attribute("type") || !strcasecmp(entity->Attribute("type"), "available")) {
				msg.what = JAB_GROUP_CHATTER_ONLINE;
			} else if (!strcasecmp(entity->Attribute("type"), "unavailable")) {
				msg.what = JAB_GROUP_CHATTER_OFFLINE;
			}

			MessageRepeater::Instance()->PostMessage(&msg);

			roster->Unlock();
			return;
		}

		for (JRoster::ConstRosterIter i = roster->BeginIterator(); i != roster->EndIterator(); ++i) {
			UserID *user = NULL;

			if ((*i)->IsUser() && !strcasecmp(UserID(entity->Attribute("from")).JabberHandle().c_str(), (*i)->JabberHandle().c_str())) {
				// found another match
				++num_matches;

				user = *i;

				_ProcessUserPresence(user, entity);
			} else if ((*i)->UserType() == UserID::TRANSPORT && !strcasecmp(UserID(entity->Attribute("from")).TransportID().c_str(), (*i)->TransportID().c_str())) {
				// found another match
				++num_matches;

				user = *i;
				_ProcessUserPresence(user, entity);
			}
		}
		
		if (num_matches == 0) {
			UserID user(entity->Attribute("from"));
			
			_ProcessUserPresence(&user, entity);
		}
			
		roster->Unlock();

		// update all RosterViews
		JRoster::Instance()->RefreshRoster();				
	}
}

void JabberSpeak::_ProcessUserPresence(UserID *user, XMLEntity *entity) {
	char buffer[4096];

	// get best asker name
	const char *asker;
					
	if (user && user->FriendlyName().size() > 0) {
		// they have a friendly name
		asker = user->FriendlyName().c_str();
	} else if (entity->Attribute("from")) {
		// they have a JID
		asker = entity->Attribute("from");
	} else {
		// they have no identity (illegal case)
		asker = "<unknown>";
	}

	// get presence
	const char *availability = NULL;

	if (entity->Attribute("type")) {
		availability = entity->Attribute("type");
	} else {
		availability = "available";
	}
				
	// reflect presence
	if (user && !strcasecmp(availability, "unavailable")) {
		user->SetOnlineStatus(UserID::OFFLINE);
	} else if (user && !strcasecmp(availability, "available")) {
		user->SetOnlineStatus(UserID::ONLINE);
	} else if (!strcasecmp(availability, "unsubscribe")) {
		sprintf(buffer, "%s no longer wishes to know your online status.", asker);
		ModalAlertFactory::NonModalAlert(buffer, "I feel so unloved.");
	} else if (user && !strcasecmp(availability, "unsubscribed")) {
		// do nothing?
		user->SetOnlineStatus(UserID::UNKNOWN);
	} else if (user && !strcasecmp(availability, "subscribed")) {
		user->SetOnlineStatus(UserID::ONLINE);

		if (entity->Child("status")) {
			sprintf(buffer, "[%s]\n\n%s", asker, entity->Child("status")->Data());
		} else {
			sprintf(buffer, "Your subscription request was accepted by %s!", asker);
		}
		
		ModalAlertFactory::Alert(buffer, "Hooray!");
	} else if (!strcasecmp(availability, "subscribe")) {
		sprintf(buffer, "%s would like to subscribe to your presence so they may know if you're online or not.  Would you like to allow it?", asker);

		// query for presence authorization (for users)
		int32 answer = 0;
				
		if (user->IsUser()) {
			answer = ModalAlertFactory::Alert(buffer, "No, I prefer privacy.", "Yes, grant them my presence!");
		} else if (user->UserType() == UserID::TRANSPORT) {
			answer = 1;
		}

		// send back the response
		if (answer == 1) {
			// presence is granted
			_AcceptPresence(entity->Attribute("from"));
		} else if (answer == 0) {
			// presence is denied
			_RejectPresence(entity->Attribute("from"));
		}
	}

	if (user && (!strcasecmp(availability, "available") || !strcasecmp(availability, "unavailable"))) {
		if (entity->Child("show") && entity->Child("show")->Data()) {
			user->SetExactOnlineStatus(entity->Child("show")->Data());
		}

		if (entity->Child("status") && entity->Child("status")->Data()) {
			user->SetMoreExactOnlineStatus(entity->Child("status")->Data());
		}
	}
}

void JabberSpeak::_ParseAgentList(XMLEntity *iq_agent_entity) {
	XMLEntity *entity = iq_agent_entity;
	
	// go one level deep to query
	if (entity->Child("query")) {
		entity = entity->Child("query");
	} else {
		return;
	}
	
	// iterate through child 'item' tags
	for (int i=0; i<entity->CountChildren(); ++i) {
		// handle the item child
		if (!strcasecmp(entity->Child(i)->Name(), "agent")) {
			// create a new agent
			Agent *agent = new Agent();

			// add data about the agent
			if (entity->Child(i)->Attribute("jid")) {
				agent->SetJID(entity->Child(i)->Attribute("jid"));
			}

			if (entity->Child(i)->Child("name")) {
				agent->SetName(entity->Child(i)->Child("name")->Data());
			}

			if (entity->Child(i)->Child("description")) {
				agent->SetDescription(entity->Child(i)->Child("description")->Data());
			}

			if (entity->Child(i)->Child("service")) {
				agent->SetService(entity->Child(i)->Child("service")->Data());
			}

			agent->SetRegisterableFlag(entity->Child(i)->Child("register"));

			if (entity->Child(i)->Child("transport")) {
				agent->SetTransport(entity->Child(i)->Child("transport")->Data());
			}

			agent->SetSearchableFlag(entity->Child(i)->Child("search"));

			// add to the AgentList BUGBUG need a return value
			AgentList::Instance()->AddAgent(agent);
		}
	}

	_got_some_agent_info = true;

	if (_got_some_agent_info && _got_some_roster_info) {
		_am_logged_in = true;
		MessageRepeater::Instance()->PostMessage(JAB_GOT_SERVER_INFO);
	}	
}

void JabberSpeak::_AcceptPresence(string username) {
	XMLEntity *entity;
	
	char **atts = CreateAttributeMemory(4);
	
	// assemble attributes
	strcpy(atts[0], "to");
	strcpy(atts[1], username.c_str());
	strcpy(atts[2], "type");
	strcpy(atts[3], "subscribed");

	entity = new XMLEntity("presence", (const char **)atts);

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts, 4);
	delete entity;
}

void JabberSpeak::_RejectPresence(string username) {
	XMLEntity *entity;
	
	char **atts = CreateAttributeMemory(4);
	
	// assemble attributes
	strcpy(atts[0], "to");
	strcpy(atts[1], username.c_str());
	strcpy(atts[2], "type");
	strcpy(atts[3], "unsubscribed");

	entity = new XMLEntity("presence", (const char **)atts);

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts, 4);
	delete entity;
}

//////////////////////////////////////////////////
// OUTGOING COMMUNICATION
//////////////////////////////////////////////////

void JabberSpeak::SendConnect(string username, string password, string realname, bool is_new_account, bool suppress_auto_connect) {
	_registering_new_account = is_new_account;

	// if there's another application instance running, suppress auto-login
	BList *teams = new BList;
	
	// query for this app signature
	be_roster->GetAppList("application/jabber", teams);
	
	if ((username.size() == 0 || password.size() == 0) && teams->CountItems() > 1) {
		suppress_auto_connect = true;
	}
	
	// if we don't have all the data, query for it
	if (username.size() == 0 || password.size() == 0 || UserID(username).JabberServer().size() == 0) {
		// check auto-login 
		if (suppress_auto_connect == false && _blabber_settings->Tag("auto-login")) {
			// last login data should be used (make sure it's there though)
			realname = (_blabber_settings->Data("last-realname")) ? _blabber_settings->Data("last-realname") : "";
			username = (_blabber_settings->Data("last-login")) ? _blabber_settings->Data("last-login") : "";
			password = (_blabber_settings->Data("last-password")) ? _blabber_settings->Data("last-password") : "";
		}

		// if we still don't have all the data, query for it
		if (username.size() == 0 || password.size() == 0 || UserID(username).JabberServer().size() == 0) {
			BlabberMainWindow::Instance()->Lock();
			BlabberMainWindow::Instance()->ShowLogin();
			BlabberMainWindow::Instance()->Unlock();
			return;
		}
	}

	// save username/password
	_curr_realname = realname;
	_curr_login    = username;
	_password      = password;

	// PLACEHOLDER
	// spawn listener thread (communication from remote machine)
	resume_thread(_connection_thread_id = spawn_thread(JabberSpeak::_SpawnConnectionThread, "connection_listener", B_LOW_PRIORITY, this));
}

int32 JabberSpeak::_SpawnConnectionThread(void *obj) {
	((JabberSpeak *)obj)->_ConnectionThread();
	
	// Don't care about the return value
	return 1;
}

void JabberSpeak::_ConnectionThread() {
	// now we have the data, let's process this user request
	XMLEntity *entity;
	char **atts = CreateAttributeMemory(6);

	string server = UserID(_curr_login).JabberServer();

	// connect to the server
	while (!IsConnected()) {
		// try to establish connection
		Connect(server.c_str(), 5222, true);

		if (IsConnected()) {
			break;
		}
		
		// wait
		snooze(200000);
	}

	strcpy(atts[0], "to");
	strcpy(atts[1], server.c_str());
	strcpy(atts[2], "xmlns");
	strcpy(atts[3], "jabber:client");
	strcpy(atts[4], "xmlns:stream");
	strcpy(atts[5], "http://etherx.jabber.org/streams"); // HARDCODE
	
	// construct XML tagset
	entity = new XMLEntity("stream:stream", (const char **)atts);

	// send XML command
	char *str = entity->StartToString();
	SendFiltered(str);
	free(str);

	// broadcast
	MessageRepeater::Instance()->PostMessage(JAB_CONNECTING);

	DestroyAttributeMemory(atts, 6);
	delete entity;
}

void JabberSpeak::SendDisconnect() {
	XMLEntity *end_stream;
	
	_am_logged_in = false;

	end_stream = new XMLEntity("stream:stream", NULL);
	
	char *str = end_stream->EndToString();
	SendFiltered(str);
	free(str);

	delete end_stream;
}

void JabberSpeak::SendSubscriptionRequest(string username) {
	XMLEntity *entity;
	
	char **atts = CreateAttributeMemory(6);
	
	// assemble attributes
	strcpy(atts[0], "to");
	strcpy(atts[1], username.c_str());
	strcpy(atts[2], "type");
	strcpy(atts[3], "subscribe");
	strcpy(atts[4], "id");
	strcpy(atts[5], GenerateUniqueID().c_str());

	entity = new XMLEntity("presence", (const char **)atts);

	// log command
	_iq_map[atts[5]] = LOGIN;
	
	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts, 6);
	delete entity;
}

void JabberSpeak::SendUnsubscriptionRequest(string username) {
	XMLEntity *entity;
	
	char **atts = CreateAttributeMemory(6);
	
	// assemble attributes
	strcpy(atts[0], "to");
	strcpy(atts[1], username.c_str());
	strcpy(atts[2], "type");
	strcpy(atts[3], "unsubscribe");
	strcpy(atts[4], "id");
	strcpy(atts[5], GenerateUniqueID().c_str());

	entity = new XMLEntity("presence", (const char **)atts);

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts, 6);
	delete entity;
}

void JabberSpeak::AddToRoster(const UserID *new_user) {
	XMLEntity *entity, *entity_query, *entity_item;
	
	char **atts       = CreateAttributeMemory(2);
	char **atts_query = CreateAttributeMemory(2);
	char **atts_item  = CreateAttributeMemory(6);
	
	// assemble attributes
	strcpy(atts[0], "type");
	strcpy(atts[1], "set");

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:roster");

	strcpy(atts_item[0], "jid");
	strcpy(atts_item[1], new_user->Handle().c_str());
	strcpy(atts_item[2], "name");
	strcpy(atts_item[3], new_user->FriendlyName().c_str());
	strcpy(atts_item[4], "subscription");
	strcpy(atts_item[5], "to");

	entity = new XMLEntity("iq", (const char **)atts);
	entity_query = new XMLEntity("query", (const char **)atts_query);
	entity_item = new XMLEntity("item", (const char **)atts_item);

	entity_query->AddChild(entity_item);
	entity->AddChild(entity_query);

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts, 2);
	DestroyAttributeMemory(atts_query, 2);
	DestroyAttributeMemory(atts_item, 6);
	
	delete entity;
}

void JabberSpeak::RemoveFromRoster(const UserID *removed_user) {
	XMLEntity *entity, *entity_query, *entity_item;

	char **atts       = CreateAttributeMemory(4);
	char **atts_query = CreateAttributeMemory(2);
	char **atts_item  = CreateAttributeMemory(6);
	
	// assemble attributes
	strcpy(atts[0], "type");
	strcpy(atts[1], "set");
	strcpy(atts[2], "id");
	strcpy(atts[3], GenerateUniqueID().c_str());

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:roster");

	strcpy(atts_item[0], "jid");
	strcpy(atts_item[1], removed_user->Handle().c_str());
	strcpy(atts_item[2], "name");
	strcpy(atts_item[3], removed_user->FriendlyName().c_str());
	strcpy(atts_item[4], "subscription");
	strcpy(atts_item[5], "remove");

	entity = new XMLEntity("iq", (const char **)atts);
	entity_query = new XMLEntity("query", (const char **)atts_query);
	entity_item = new XMLEntity("item", (const char **)atts_item);

	entity_query->AddChild(entity_item);
	entity->AddChild(entity_query);

	// log command
	_iq_map[atts[3]] = ROSTER;

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);
	
	DestroyAttributeMemory(atts, 4);
	DestroyAttributeMemory(atts_query, 2);
	DestroyAttributeMemory(atts_item, 6);
	
	delete entity;
}

void JabberSpeak::SendMessage(const TalkWindow::talk_type type, const UserID *user, string message, string thread_id) {
	XMLEntity   *entity;
	char **atts = CreateAttributeMemory(4);

	// assemble attributes;
	strcpy(atts[0], "to");
	strcpy(atts[1], user->Handle().c_str());
	strcpy(atts[2], "type");

	if (type == TalkWindow::CHAT) {
		strcpy(atts[3], "chat");
	} else {
		strcpy(atts[3], "normal");
	}
	
	// construct XML tagset
	entity = new XMLEntity("message", (const char **)atts);

	entity->AddChild("body", NULL, message.c_str());
	entity->AddChild("thread", NULL, thread_id.c_str());

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts, 4);
	
	delete entity;
}

void JabberSpeak::SendMessage(const TalkWindow::talk_type type, string group_room, string message) {
	XMLEntity   *entity;
	char **atts = CreateAttributeMemory(4);

	// assemble attributes;
	strcpy(atts[0], "to");
	strcpy(atts[1], group_room.c_str());
	strcpy(atts[2], "type");
	strcpy(atts[3], "groupchat");
	
	// construct XML tagset
	entity = new XMLEntity("message", (const char **)atts);

	entity->AddChild("body", NULL, message.c_str());

	// send XML command
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts, 4);
	
	delete entity;
}

void JabberSpeak::SendPresence(string show, string status) {
	if (show.empty()) {
		// quick version
		SendFiltered("<presence/>");
	} else {
		// detailed version
		XMLEntity *entity = new XMLEntity("presence", NULL);
		
		// add show information
		entity->AddChild("show", NULL, show.c_str());

		// add status information
		if (!status.empty()) {
			entity->AddChild("status", NULL, status.c_str());
		}

		char *str = entity->ToString();
		SendFiltered(str);
		free(str);

		delete entity;
	}
}

void JabberSpeak::SendLastPresence() {
	bool last_used_custom   = BlabberSettings::Instance()->Tag("last-used-custom-status");
	const char *last_status = BlabberSettings::Instance()->Data("last-custom-exact-status");
	const char *last_custom_status = BlabberSettings::Instance()->Data("last-custom-more-exact-status");

	// do we have statuses?
	if (last_status) {
		if (last_used_custom) {
			SendPresence(last_status, last_custom_status);
			BlabberMainWindow::Instance()->SetCustomStatus(last_custom_status);
		} else {
			if (!strcasecmp(last_status, "chat")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_AVAILABLE_FOR_CHAT);
			} else if (!strcasecmp(last_status, "away")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_AWAY_TEMPORARILY);
			} else if (!strcasecmp(last_status, "xa")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_AWAY_EXTENDED);
			} else if (!strcasecmp(last_status, "dnd")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_DO_NOT_DISTURB);
			} else if (!strcasecmp(last_status, "school")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_SCHOOL);
			} else if (!strcasecmp(last_status, "work")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_WORK);
			} else if (!strcasecmp(last_status, "lunch")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_LUNCH);
			} else if (!strcasecmp(last_status, "dinner")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_DINNER);
			} else if (!strcasecmp(last_status, "sleep")) {
				BlabberMainWindow::Instance()->PostMessage(BLAB_SLEEP);
			} else {
				BlabberMainWindow::Instance()->PostMessage(BLAB_AVAILABLE_FOR_CHAT);
			}
		}
	} else {
		BlabberMainWindow::Instance()->PostMessage(BLAB_AVAILABLE_FOR_CHAT);
	}
}

void JabberSpeak::SendGroupPresence(string _group_room, string _group_username) {
	XMLEntity             *entity_presence;
	char **atts_presence = CreateAttributeMemory(2);

	// assemble group ID
	string group_presence = _group_room + "/" + _group_username;	
	
	// assemble attributes;
	strcpy(atts_presence[0], "to");
	strcpy(atts_presence[1], group_presence.c_str());

	// construct XML tagset
	entity_presence = new XMLEntity("presence", (const char **)atts_presence);
	
	// send XML command
	char *str = entity_presence->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts_presence, 2);
	
	delete entity_presence;
}

void JabberSpeak::SendGroupUnvitation(string _group_room, string _group_username) {
	XMLEntity             *entity_presence;
	char **atts_presence = CreateAttributeMemory(4);

	// assemble group ID
	string group_presence = _group_room + "/" + _group_username;	
	
	// assemble attributes;
	strcpy(atts_presence[0], "to");
	strcpy(atts_presence[1], group_presence.c_str());
	strcpy(atts_presence[2], "type");
	strcpy(atts_presence[3], "unavailable");

	// construct XML tagset
	entity_presence = new XMLEntity("presence", (const char **)atts_presence);
	
	// send XML command
	char *str = entity_presence->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts_presence, 4);
	
	delete entity_presence;
}

void JabberSpeak::_SendUserRegistration(string username, string password, string resource) {
	XMLEntity   *entity_iq, *entity_query;
	char **atts_iq    = CreateAttributeMemory(4);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts_iq[0], "id");

	strcpy(atts_iq[1], GenerateUniqueID().c_str());

	strcpy(atts_iq[2], "type");
	strcpy(atts_iq[3], "set");

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:register");

	// construct XML tagset
	entity_iq    = new XMLEntity("iq", (const char **)atts_iq);
	entity_query = new XMLEntity("query", (const char **)atts_query);

	entity_iq->AddChild(entity_query);
	
	entity_query->AddChild("username", NULL, username.c_str());
	entity_query->AddChild("password", NULL, password.c_str());
	entity_query->AddChild("resource", NULL, resource.c_str());

	// log command
	_iq_map[atts_iq[1]] = NEW_USER;
	
	// send XML command
	char *str = entity_iq->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts_iq, 4);
	DestroyAttributeMemory(atts_query, 2);
	
	delete entity_iq;
}

void JabberSpeak::RegisterWithAgent(string agent) {
	XMLEntity          *entity;
	char **atts       = CreateAttributeMemory(6);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts[0], "type");
	strcpy(atts[1], "get");

	strcpy(atts[2], "id");
	strcpy(atts[3], GenerateUniqueID().c_str());

	strcpy(atts[4], "to");
	strcpy(atts[5], AgentList::Instance()->GetAgentByService(agent)->JID().c_str());

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:register");

	// send XML command
	entity = new XMLEntity("iq", (const char **)atts);
	entity->AddChild("query", (const char **)atts_query, NULL);

	// log command
	_iq_map[atts[3]] = REGISTER;
	
	char *str = entity->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts, 6);
	DestroyAttributeMemory(atts_query, 2);
	
	delete entity;
}

void JabberSpeak::UnregisterWithAgent(string agent) {
	// Find registration in roster
	JRoster::Instance()->Lock();

	UserID *user = NULL;
	if (AgentList::Instance()->GetAgentByService(agent)) {
		user = JRoster::Instance()->FindUser(JRoster::TRANSPORT_ID, AgentList::Instance()->GetAgentByService(agent)->JID());
	}
	
	// call RemoveFromRoster()
	if (user) {
		RemoveFromRoster(user);
	}

	JRoster::Instance()->Unlock();
}

void JabberSpeak::_SendAuthentication(string username, string password, string resource) {
	XMLEntity   *entity_iq, *entity_query;
	char **atts_iq    = CreateAttributeMemory(4);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts_iq[0], "id");

	strcpy(atts_iq[1], GenerateUniqueID().c_str());

	strcpy(atts_iq[2], "type");
	strcpy(atts_iq[3], "set");

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:auth");

	// construct XML tagset
	entity_iq    = new XMLEntity("iq", (const char **)atts_iq);
	entity_query = new XMLEntity("query", (const char **)atts_query);

	entity_iq->AddChild(entity_query);
	
	entity_query->AddChild("username", NULL, username.c_str());
	entity_query->AddChild("password", NULL, password.c_str());
	entity_query->AddChild("resource", NULL, resource.c_str());

	// log command
	_iq_map[atts_iq[1]] = LOGIN;
	
	// send XML command
	char *str = entity_iq->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts_iq, 4);
	DestroyAttributeMemory(atts_query, 2);
	
	delete entity_iq;
}

void JabberSpeak::_SendAgentRequest() {
	XMLEntity   *entity_agent;
	char **atts = CreateAttributeMemory(4);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts[0], "type");
	strcpy(atts[1], "get");

	strcpy(atts[2], "id");
	strcpy(atts[3], GenerateUniqueID().c_str());

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:agents");

	// send XML command
	entity_agent = new XMLEntity("iq", (const char **)atts);
	entity_agent->AddChild("query", (const char **)atts_query, NULL);

	// log command
	_iq_map[atts[3]] = AGENTS;
	
	char *str = entity_agent->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts, 4);
	DestroyAttributeMemory(atts_query, 2);
	
	delete entity_agent;
}

void JabberSpeak::_SendRosterRequest() {
	XMLEntity   *entity_roster;
	char **atts = CreateAttributeMemory(4);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes
	strcpy(atts[0], "type");
	strcpy(atts[1], "get");

	strcpy(atts[2], "id");
	strcpy(atts[3], GenerateUniqueID().c_str());

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:roster");

	// send XML command
	entity_roster = new XMLEntity("iq", (const char **)atts);
	entity_roster->AddChild("query", (const char **)atts_query, NULL);
	
	// log command
	_iq_map[atts[3]] = ROSTER;

	char *str = entity_roster->ToString();
	SendFiltered(str);
	free(str);

	DestroyAttributeMemory(atts, 4);
	DestroyAttributeMemory(atts_query, 2);
	
	delete entity_roster;
}
