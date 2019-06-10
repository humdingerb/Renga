//////////////////////////////////////////////////
// Blabber [JabberSpeak.cpp]
//////////////////////////////////////////////////

#include <gloox/jid.h>
#include <gloox/rostermanager.h>

#include "JabberSpeak.h"
#include <cstdio>
#include <Roster.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "BlabberApp.h"
#include "AgentList.h"
#include "GenericFunctions.h"
#include "JRoster.h"
#include "ModalAlertFactory.h"
#include "MessageRepeater.h"
#include "Messages.h"
#include "TalkManager.h"
#include "UserID.h"
#include "XMLEntity.h"

#include <stdlib.h>

#include <Path.h>
#include <FindDirectory.h>
#include <AppFileInfo.h>
#include <File.h>

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
	: XMLReader() {
	_registering_new_account = false;
	
	// grab a handle to the settings now for convenience later
	_blabber_settings = BlabberSettings::Instance();
}

JabberSpeak::~JabberSpeak() {
	_instance = NULL;
}

void JabberSpeak::Reset() {
	if (!_reconnecting) {
		BlabberMainWindow::Instance()->Lock();
		BlabberMainWindow::Instance()->ShowLogin();
		BlabberMainWindow::Instance()->Unlock();
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
}

void JabberSpeak::JabberSpeakReset() {
	_curr_realname           = "";
	_curr_login              = "";
	_password                = "";
	_ssl_server				 = "";
	_ssl_port				 = 0;
	_ssl_enabled			 = false;
	_am_logged_in            = false;
	_registering_new_account = false;
	_reconnecting            = false;
	_got_some_agent_info     = false;
	_got_some_roster_info    = false;

	_iq_map.clear();
}

//////////////////////////////////////////////////
// STANDARD METHODS
//////////////////////////////////////////////////

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

//////////////////////////////////////////////////
// INCOMING COMMUNICATION
//////////////////////////////////////////////////

void JabberSpeak::OnTag(XMLEntity *entity) {
	char buffer[4096]; // general buffer space
	string iq_id;      // used for IQ tags

	static int seen_streams = 0;

	// handle connection
	if (!entity->IsCompleted() && !strcasecmp(entity->Name(), "stream:stream")) {
		if (_registering_new_account) {
			// create account
			_SendUserRegistration(UserID(_curr_login).JabberUsername(), _password, UserID(_curr_login).JabberResource());
		}
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

		// handle verion request
		if (!strcasecmp(entity->Attribute("type"), "get")) {
			string iq_from;   
			if (entity->Attribute("from")) {
				iq_from = entity->Attribute("from");
			}
		
			// get access to query tag
			XMLEntity *query = entity->Child("query");
			if (query && query->Attribute("xmlns")) {
				if (!strcasecmp(query->Attribute("xmlns"), "jabber:iq:version")) {
					_ProcessVersionRequest(iq_id, iq_from);
				}
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
	free(str);
	
	DestroyAttributeMemory(atts_iq, 6);
	DestroyAttributeMemory(atts_query, 2);
	delete entity_iq;
}

void JabberSpeak::_ProcessUserPresence(UserID *user,
	gloox::Presence::PresenceType type, const std::string& message) {
#if 0
	// get best asker name
	const char *asker;
					
	if (user && user->FriendlyName().size() > 0) {
		// they have a friendly name
		asker = user->FriendlyName().c_str();
	} else {
		// they have a JID
		asker = entity.from().full();
	}full
#endif

	// reflect presence
	if (user && type == gloox::Presence::Unavailable) {
		user->SetOnlineStatus(UserID::OFFLINE);
	} else if (user && type == gloox::Presence::Available) {
		user->SetOnlineStatus(UserID::ONLINE);
#if 0
	// FIXME we should rather implement RosterManager instead of doing this ourselves
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
#endif
	}

	if (user && (type == gloox::Presence::Unavailable
			|| type == gloox::Presence::Available)) {
#if 0
		if (entity->Child("show") && entity->Child("show")->Data()) {
			user->SetExactOnlineStatus(entity->Child("show")->Data());
		}
#endif

		user->SetMoreExactOnlineStatus(message);
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
	free(str);
	
	DestroyAttributeMemory(atts, 4);
	delete entity;
}

//////////////////////////////////////////////////
// OUTGOING COMMUNICATION
//////////////////////////////////////////////////

void JabberSpeak::SendConnect(string username, string password, string realname, bool ssl_enabled, string ssl_server, int32 ssl_port, bool is_new_account, bool suppress_auto_connect) {
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
			ssl_server = (_blabber_settings->Data("last-ssl_server")) ? _blabber_settings->Data("last-ssl_server") : "";
			ssl_enabled = (_blabber_settings->Data("last-ssl_enabled")) ? atoi(_blabber_settings->Data("last-ssl_enabled")) : false;
			ssl_port = (_blabber_settings->Data("last-ssl_port")) ? atoi(_blabber_settings->Data("last-ssl_port")) : 0;		
		}

		// if we still don't have all the data, query for it
		if (username.size() == 0 || password.size() == 0 || UserID(username).JabberServer().size() == 0
			|| (ssl_enabled && ( ssl_server.size() == 0 || ssl_port <= 0) ) ) {
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
	_ssl_server	   = ssl_server;
	_ssl_port	   = ssl_port;
	_ssl_enabled   = ssl_enabled;

	// PLACEHOLDER
	// spawn listener thread (communication from remote machine)
	resume_thread(_connection_thread_id = spawn_thread(JabberSpeak::_SpawnConnectionThread, "connection_listener", B_LOW_PRIORITY, this));
}


int32 JabberSpeak::_SpawnConnectionThread(void *obj) {
	((JabberSpeak *)obj)->_ConnectionThread();

	// Don't care about the return value
	return 1;
}


string					
JabberSpeak::GetRealServer()
{
	if (_ssl_enabled && _ssl_server.size() >0 && _ssl_port >0 )
	{
		return _ssl_server;
	}
	
	return UserID(_curr_login).JabberServer();
}

int	
JabberSpeak::GetRealPort()
{
	if (_ssl_enabled && _ssl_server.size() >0 && _ssl_port >0 )
	{
		return _ssl_port;
	}
	
	return 5222; //default jabber port.
}	


void JabberSpeak::_ConnectionThread() {
	gloox::JID jid(_curr_login);
	fClient = new gloox::Client(jid, _password);
	fClient->registerConnectionListener(this);
	fClient->rosterManager()->registerRosterListener(this);
	fClient->registerMessageHandler(TalkManager::Instance());
	puts("connect");
	fClient->connect();
	puts("connect ok");
}


void JabberSpeak::SendDisconnect() {
	XMLEntity *end_stream;
	
	_am_logged_in = false;

	end_stream = new XMLEntity("stream:stream", NULL);
	
	char *str = end_stream->EndToString();
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
	free(str);
	
	DestroyAttributeMemory(atts, 4);
	DestroyAttributeMemory(atts_query, 2);
	DestroyAttributeMemory(atts_item, 6);
	
	delete entity;
}


void JabberSpeak::SendMessage(const TalkWindow::talk_type type,
	const UserID *user, string body, string thread_id)
{
	gloox::Message message(type == TalkWindow::CHAT ?
			gloox::Message::Chat : gloox::Message::Normal,
		gloox::JID(user->JabberCompleteHandle()), body, gloox::EmptyString,
		thread_id);
	fClient->send(message);
}


void JabberSpeak::SendMessage(__attribute__((unused)) const TalkWindow::talk_type type, __attribute__((unused)) string group_room, __attribute__((unused)) string message) {
	puts(__PRETTY_FUNCTION__);
}

void JabberSpeak::SendPresence(string show, string status) {
	if (show.empty()) {
		// quick version
		//SendFiltered("<presence/>");
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

void JabberSpeak::_ProcessVersionRequest(string req_id, string req_from) {
	XMLEntity   *entity_iq, *entity_query;
	char **atts_iq    = CreateAttributeMemory(6);
	char **atts_query = CreateAttributeMemory(2);

	// assemble attributes;
	strcpy(atts_iq[0], "id");
	strcpy(atts_iq[1], req_id.c_str());

	strcpy(atts_iq[2], "to");
	strcpy(atts_iq[3], req_from.c_str());

	strcpy(atts_iq[4], "type");
	strcpy(atts_iq[5], "result");

	strcpy(atts_query[0], "xmlns");
	strcpy(atts_query[1], "jabber:iq:version");

	// construct XML tagset
	entity_iq    = new XMLEntity("iq", (const char **)atts_iq);
	entity_query = new XMLEntity("query", (const char **)atts_query);

	entity_iq->AddChild(entity_query);
	
	entity_query->AddChild("name", NULL, "Jabber");
	entity_query->AddChild("version", NULL, APP_VERSION);

	string strVersion("Haiku");
	BPath path;
	if (find_directory(B_BEOS_LIB_DIRECTORY, &path) == B_OK) {
		path.Append("libbe.so");

		BAppFileInfo appFileInfo;
		version_info versionInfo;
		BFile file;
		if (file.SetTo(path.Path(), B_READ_ONLY) == B_OK
			&& appFileInfo.SetTo(&file) == B_OK
			&& appFileInfo.GetVersionInfo(&versionInfo, 
				B_APP_VERSION_KIND) == B_OK
			&& versionInfo.short_info[0] != '\0')
				strVersion = versionInfo.short_info;
	}

	string os_info;
	utsname uname_info;
	if (uname(&uname_info) == 0) {
		os_info = uname_info.sysname;
		long revision = 0;
		if (sscanf(uname_info.version, "r%ld", &revision) == 1) {
			char version[16];
			snprintf(version, sizeof(version), "%ld", revision);
			os_info += " ( " + strVersion + " Rev. ";
			os_info += version;
			os_info += ")";
		}
	}

	entity_query->AddChild("os", NULL, os_info.c_str());

	// send XML command
	char *str = entity_iq->ToString();
	free(str);

	DestroyAttributeMemory(atts_iq, 6);
	DestroyAttributeMemory(atts_query, 2);
	
	delete entity_iq;
}

void
JabberSpeak::onConnect()
{
	MessageRepeater::Instance()->PostMessage(JAB_LOGGED_IN);
	_reconnecting = false;
	//SendLastPresence();	
}

void
JabberSpeak::onDisconnect(__attribute__((unused)) gloox::ConnectionError e)
{
	puts("onDisconnect");
	
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
		Reset();

		SendConnect();
	}
}

bool
JabberSpeak::onTLSConnect(__attribute__((unused)) const gloox::CertInfo& info)
{
	// TODO verify certificate
	return true;
}


void
JabberSpeak::handleItemAdded(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JabberSpeak::handleItemSubscribed(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JabberSpeak::handleItemRemoved(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JabberSpeak::handleItemUpdated(const gloox::JID& jid)
{
	printf("%s(%s)\n", __PRETTY_FUNCTION__, jid.full().c_str());
}


void
JabberSpeak::handleItemUnsubscribed(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JabberSpeak::handleRoster(const gloox::Roster& roster)
{
	JRoster::Instance()->Lock();

	for (auto item: roster) {
		UserID user(item.first);
		user.SetFriendlyName(item.second->name());
		user.SetSubscriptionStatus(item.second->subscription());
		// todo user.SetAsk(???)

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
#if 0
			// process if it's a removal
			if (entity->Child(i)->Attribute("subscription") && !strcasecmp(entity->Child(i)->Attribute("subscription"), "remove")) {
				// remove from the list
				JRoster::Instance()->RemoveUser(roster_user);

				continue;
			}
#endif

			// update the new roster item
			*roster_user = user;
		} else {
			// create the user
			roster_user = new UserID(item.first);

			*roster_user = user;

			// add to the list
			JRoster::Instance()->AddRosterUser(roster_user);
		}
	}

	JRoster::Instance()->Unlock();	

	// update all RosterViews
	JRoster::Instance()->RefreshRoster();
	
	_got_some_roster_info = true;
	
	if (_got_some_agent_info && _got_some_roster_info) {
		_am_logged_in = true;
	}
}


void
JabberSpeak::handleRosterPresence(const gloox::RosterItem& item,
	const string& resource, gloox::Presence::PresenceType presenceType,
	const string& message)
{
	JRoster *roster = JRoster::Instance();
	int num_matches = 0;
	const gloox::JID& jid = item.jidJID();

	roster->Lock();

	if (resource != "" && !TalkManager::Instance()->IsExistingWindowToGroup(
		TalkWindow::GROUP, jid.bare()).empty())
	{
		BMessage msg;
		msg.AddString("room", jid.bare().c_str());
		msg.AddString("server", jid.server().c_str());
		msg.AddString("username", jid.resource().c_str());

		if (presenceType == gloox::Presence::Available) {
			msg.what = JAB_GROUP_CHATTER_ONLINE;
		} else if (presenceType == gloox::Presence::Unavailable) {
			msg.what = JAB_GROUP_CHATTER_OFFLINE;
		}
		MessageRepeater::Instance()->PostMessage(&msg);
		roster->Unlock();
		return;
	}
	
	for (JRoster::ConstRosterIter i = roster->BeginIterator();
		i != roster->EndIterator(); ++i)
	{
		UserID *user = NULL;
		if ((*i)->IsUser() && !strcasecmp(UserID(jid.full()).JabberHandle().c_str(),
			(*i)->JabberHandle().c_str()))
		{
			// found another match
			++num_matches;
			user = *i;
			_ProcessUserPresence(user, presenceType, message);
		} else if ((*i)->UserType() == UserID::TRANSPORT
			&& !strcasecmp(UserID(jid.full()).TransportID().c_str(),
				(*i)->TransportID().c_str()))
		{
			// found another match
			++num_matches;
			user = *i;
			_ProcessUserPresence(user, presenceType, message);
		}
	}
	if (num_matches == 0) {
#if 0
		UserID user(jid.full());
		ProcessUserPresence(&user, presence);
#endif
		puts("user not found");
	}

	roster->Unlock();

	// update all RosterViews
	JRoster::Instance()->RefreshRoster();				
}


void
JabberSpeak::handleSelfPresence(const gloox::RosterItem&, const string&, gloox::Presence::PresenceType, const string&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


bool
JabberSpeak::handleSubscriptionRequest(const gloox::JID&, const string&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
	return false;
}


bool
JabberSpeak::handleUnsubscriptionRequest(const gloox::JID&, const string&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
	return false;
}


void
JabberSpeak::handleNonrosterPresence(const gloox::Presence&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JabberSpeak::handleRosterError(const gloox::IQ&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}
