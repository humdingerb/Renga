//////////////////////////////////////////////////
// Blabber [JRoster.cpp]
//////////////////////////////////////////////////

#include <kernel/OS.h>

#include <gloox/rostermanager.h>

#include "JabberSpeak.h"
#include "JRoster.h"
#include "Messages.h"
#include "MessageRepeater.h"

JRoster *JRoster::_instance = NULL;

JRoster *JRoster::Instance() {
	if (_instance == NULL) {
		_instance = new JRoster();
	}
	
	return _instance;	
}

JRoster::~JRoster() {
	_instance = NULL;

	// destroy semaphore
	delete_sem(_roster_lock);
}
	    
void JRoster::AddRosterUser(UserID *roster_user) {
	_roster->push_back(roster_user);
	
	// refresh all roster views
	RefreshRoster();
}

void JRoster::AddNewUser(UserID *new_user) {
	_roster->push_back(new_user);

	// communicate the new user to the server
	JabberSpeak::Instance()->AddToRoster(new_user);

	// refresh all roster views
	RefreshRoster();
}

void JRoster::RemoveUser(const UserID *removed_user) {
	for (RosterIter i = _roster->begin(); i != _roster->end(); ++i) {
		if (*i == removed_user) {
			_roster->erase(i);

			// for transports
			if (removed_user->UserType() == UserID::TRANSPORT) {
				Agent *agent = AgentList::Instance()->GetAgentByID(removed_user->TransportID());

				if (agent) {
					agent->SetRegisteredFlag(false);
				}
			}
			
			// goodbye memory
			delete removed_user;

			break;	
		}
	}
}

void JRoster::RemoveAllUsers() {
	// BUGBUG need more elegant way of deleting all users (check STL guide)
	while(_roster->begin() != _roster->end()) {
		RosterIter i = _roster->begin();
		UserID *user = *i;

		_roster->erase(i);
		delete user;
	}
}
	
UserID *JRoster::FindUser(search_method search_type, string name) {
	if (search_type == JRoster::FRIENDLY_NAME) {
		for (RosterIter i = _roster->begin(); i != _roster->end(); ++i) {
			if (!strcasecmp(name.c_str(), (*i)->FriendlyName().c_str())) {
				return (*i);
			}
		}
	}
	
	if (search_type == JRoster::HANDLE) {
		for (RosterIter i = _roster->begin(); i != _roster->end(); ++i) {
			if (!strcasecmp(name.c_str(), (*i)->JabberHandle().c_str())) {
				return (*i);
			}
		}
	}

	if (search_type == JRoster::COMPLETE_HANDLE) {
		for (RosterIter i = _roster->begin(); i != _roster->end(); ++i) {
			if (!strcasecmp(name.c_str(), (*i)->JabberCompleteHandle().c_str())) {
				return (*i);
			}
		}
	}

	if (search_type == JRoster::TRANSPORT_ID) {
		for (RosterIter i = _roster->begin(); i != _roster->end(); ++i) {
			if (!strcasecmp(name.c_str(), (*i)->TransportID().c_str())) {
				return (*i);
			}
		}
	}

	return NULL;
}

bool JRoster::ExistingUserObject(const UserID *comparing_user) {
	for (RosterIter i = _roster->begin(); i != _roster->end(); ++i) {
		if ((*i) == comparing_user) {
			return true;
		}
	}
	
	return false;
}

UserID *JRoster::FindUser(const UserID *comparing_user) {
	return FindUser(JRoster::HANDLE, comparing_user->JabberHandle());
}

void JRoster::SetUserStatus(string username, UserID::online_status status) {
	UserID *user = const_cast<UserID *>(FindUser(COMPLETE_HANDLE, username));
	
	if (user != NULL) {
		user->SetOnlineStatus(status);
	}

	// refresh all roster views
	RefreshRoster();
}

UserID::online_status JRoster::UserStatus(string username) {
	UserID *user = FindUser(COMPLETE_HANDLE, username);
	
	if (user != NULL) {
		return user->OnlineStatus();
	}

	return UserID::UNKNOWN;
}

JRoster::ConstRosterIter JRoster::BeginIterator() {
	return _roster->begin();
}

JRoster::ConstRosterIter JRoster::EndIterator() {
	return _roster->end();
}

void JRoster::RefreshRoster() {
	// update everyone to the change
	MessageRepeater::Instance()->PostMessage(BLAB_UPDATE_ROSTER);
	MessageRepeater::Instance()->PostMessage(TRANSPORT_UPDATE);
}

void JRoster::Lock() {
	acquire_sem(_roster_lock);
}

void JRoster::Unlock() {
	release_sem(_roster_lock);
}

void
JRoster::handleItemAdded(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JRoster::handleItemSubscribed(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JRoster::handleItemRemoved(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JRoster::handleItemUpdated(const gloox::JID& jid)
{
	gloox::Client* client = JabberSpeak::Instance()->GlooxClient();
	gloox::RosterItem* item = client->rosterManager()->getRosterItem(jid);
	UserID* roster_user = FindUser(JRoster::HANDLE, jid.full());
	if (roster_user) {
		roster_user->SetSubscriptionStatus(item->subscription());
		printf("%s(%s)\n", __PRETTY_FUNCTION__, jid.full().c_str());
		printf("   name %s\n", item->name().c_str());
		printf("   onln %d\n", item->online());
	} else {
		printf("%s(%s) user not found\n", __PRETTY_FUNCTION__, jid.full().c_str());
	}
}


void
JRoster::handleItemUnsubscribed(const gloox::JID&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JRoster::handleRoster(const gloox::Roster& roster)
{
	Lock();

	for (auto item: roster) {
		UserID user(item.first);
		user.SetFriendlyName(item.second->name());
		user.SetSubscriptionStatus(item.second->subscription());

		// obtain a handle to the user (is there a new one?)
		UserID *roster_user;
			
		if (user.IsUser()) {
			roster_user = FindUser(JRoster::HANDLE, user.JabberHandle());
		} else if (user.UserType() == UserID::TRANSPORT) {
			roster_user = FindUser(JRoster::TRANSPORT_ID, user.TransportID());
		} else {
			continue;
		}

		// if we have duplicates, settle disputes
		if (roster_user) {
#if 0
			// process if it's a removal
			if (entity->Child(i)->Attribute("subscription") && !strcasecmp(entity->Child(i)->Attribute("subscription"), "remove")) {
				// remove from the list
				RemoveUser(roster_user);

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
			AddRosterUser(roster_user);
		}
	}

	Unlock();	

	// update all RosterViews
	RefreshRoster();
}


void
JRoster::handleRosterPresence(const gloox::RosterItem& item,
	const string& resource __attribute__((unused)), gloox::Presence::PresenceType presenceType,
	const string& message)
{
	int num_matches = 0;
	const gloox::JID& jid = item.jidJID();

	Lock();

	for (JRoster::ConstRosterIter i = BeginIterator();
		i != EndIterator(); ++i)
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

	Unlock();

	// update all RosterViews
	RefreshRoster();				
}


void
JRoster::handleSelfPresence(const gloox::RosterItem&, const string& resource, gloox::Presence::PresenceType, const string& msg)
{
	// TODO this gets called for giving us our current presence and message,
	// and also for all other resources, so we can know that the same user
	// is also online elsewhere
	printf("%s(%s, %s)\n", __PRETTY_FUNCTION__, resource.c_str(), msg.c_str());
}


bool
JRoster::handleSubscriptionRequest(const gloox::JID&, const string&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
	return false;
}


bool
JRoster::handleUnsubscriptionRequest(const gloox::JID&, const string&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
	return false;
}


void
JRoster::handleNonrosterPresence(const gloox::Presence&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}


void
JRoster::handleRosterError(const gloox::IQ&)
{
	printf("%s\n", __PRETTY_FUNCTION__);
}

JRoster::JRoster() {
	_roster = new RosterList;
	
	// create semaphore
	_roster_lock = create_sem(1, "roster sempahore");
}

void JRoster::_ProcessUserPresence(UserID *user,
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

