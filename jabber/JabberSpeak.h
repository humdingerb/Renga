//////////////////////////////////////////////////
// Blabber [JabberSpeak.h]
//     This monolith talks the jabber language!
//////////////////////////////////////////////////

#ifndef JABBER_SPEAK_H
#define JABBER_SPEAK_H

#include <gloox/bookmarkhandler.h>
#include <gloox/bookmarkstorage.h>
#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/presence.h>
#include <gloox/rosterlistener.h>
#include <gloox/softwareversion.h>

#include <map>
#include <string>
#include <Looper.h>
#include "Agent.h"
#include "AgentList.h"
#include "BlabberSettings.h"
#include "TalkWindow.h"
#include "UserID.h"
#include "XMLReader.h"

class JabberSpeak : public XMLReader,
	public gloox::ConnectionListener, public gloox::RosterListener,
	public gloox::BookmarkHandler
{
public:
	enum iq_intent {LOGIN, ROSTER, AGENTS, REGISTER, SEND_REGISTER, UNREGISTER, SEND_UNREGISTER, NEW_USER, MESSAGE, CHAT};

	typedef map<string, iq_intent>                  IQMap;
	typedef map<string, iq_intent>::iterator        IQIter;
	typedef map<string, iq_intent>::const_iterator  ConstIQIter;

public:
	// CREATORS
	static JabberSpeak      *Instance();
	                        ~JabberSpeak();

	void                     Reset();
	void                     JabberSpeakReset();
 
	// OUTGOING COMMUNICATION
	char                   **CreateAttributeMemory(int num_items);
	void                     DestroyAttributeMemory(char **atts, int num_items);
	static string            GenerateUniqueID();
	void                     SendConnect(string username = "", string password = "", string realname = "", bool ssl_enabled = false, string ssl_server = "", int32 ssl_port = 0, bool is_new_account = false, bool suppress_auto_connect = false);
	void                     SendDisconnect();
	void                     SendSubscriptionRequest(string username); 
	void                     SendUnsubscriptionRequest(string username);
	void                     AddToRoster(const UserID *new_user);
	void                     RemoveFromRoster(const UserID *removed_user);
	void                     SendMessage(const TalkWindow::talk_type type, const UserID *user, string message, string thread_id);
	void                     SendMessage(const TalkWindow::talk_type type, string group_room, string message);
	void                     SendPresence(gloox::Presence::PresenceType = gloox::Presence::Probe, string status = "");
	void                     SendLastPresence();
	void	                 SendGroupUnvitation(string _group_room, string _group_username);

	void                     _SendUserRegistration(string username, string password, string resource);
	void                     RegisterWithAgent(string agent);
	void	                 UnregisterWithAgent(string agent);
	
	// INCOMING COMMUNICATION
	void                     OnTag(XMLEntity *entity);

	// SELECTORS
	const string             CurrentRealName() const;
	const string             CurrentLogin() const;

	string					GetRealServer();
	int						GetRealPort();

	// GLOOX LISTENERS
	void					onConnect() final;
	void					onDisconnect(gloox::ConnectionError e) final;
	bool					onTLSConnect(const gloox::CertInfo& info) final;

	void 					handleItemAdded(const gloox::JID&) final;
	void 					handleItemSubscribed(const gloox::JID&) final;
	void 					handleItemRemoved(const gloox::JID&) final;
	void 					handleItemUpdated(const gloox::JID&) final;
	void 					handleItemUnsubscribed(const gloox::JID&) final;
	void 					handleRoster(const gloox::Roster&) final;
	void 					handleRosterPresence(const gloox::RosterItem&, const string&, gloox::Presence::PresenceType, const string&) final;
	void 					handleSelfPresence(const gloox::RosterItem&, const string&, gloox::Presence::PresenceType, const string&) final;
	bool 					handleSubscriptionRequest(const gloox::JID&, const string&) final;
	bool 					handleUnsubscriptionRequest(const gloox::JID&, const string&) final;
	void 					handleNonrosterPresence(const gloox::Presence&) final;
	void 					handleRosterError(const gloox::IQ&) final;

	void 					handleBookmarks(const gloox::BookmarkList& bList,
										    const gloox::ConferenceList& cList) final;
	gloox::Client* 			GlooxClient() { return fClient; }

protected:
	// CREATORS
	                         JabberSpeak();
	                    
private:
	static int32            _SpawnConnectionThread(void *obj);
	void                    _ConnectionThread();

	// OUTGOING COMMUNICATION
	void					_ProcessVersionRequest(void);

	// INCOMING COMMUNICATION
	void                    _ProcessRegistration(XMLEntity *iq_register_entity);
	void                    _ProcessUnregistration(XMLEntity *iq_register_entity);
	void                    _SendTransportRegistrationInformation(Agent *agent, string key);
	void                    _SendTransportUnregistrationInformation(Agent *agent, string key);
	void                    _ProcessUserPresence(UserID *user, const gloox::Presence::PresenceType, const std::string&);
	void                    _ParseAgentList(XMLEntity *iq_agent_entity);
	void                    _AcceptPresence(string username);
	void                    _RejectPresence(string username);
	
	// pointer to the singleton
	BlabberSettings        *_blabber_settings;
	
	// some information about the current login status
	string                  _curr_realname;
	string                  _curr_login;
	string                  _password;
	
	string					_ssl_server;
	int32					_ssl_port;
	bool					_ssl_enabled;
	
	bool                    _am_logged_in;
	bool                    _registering_new_account;
	bool                    _reconnecting;

	bool                    _got_some_agent_info;
	bool                    _got_some_roster_info;
	
	// communication state information
	IQMap                   _iq_map;

	// CREATORS
	static JabberSpeak      *_instance;

	// threading
	thread_id                _connection_thread_id;

	gloox::Client*			fClient;
	gloox::BookmarkStorage* fBookmarks;
};

#endif
