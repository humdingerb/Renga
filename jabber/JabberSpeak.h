//////////////////////////////////////////////////
// Blabber [JabberSpeak.h]
//     This monolith talks the jabber language!
//////////////////////////////////////////////////

#ifndef JABBER_SPEAK_H
#define JABBER_SPEAK_H

#include <gloox/bookmarkstorage.h>
#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/presence.h>
#include <gloox/registration.h>
#include <gloox/softwareversion.h>

#include "../network/BookmarkManager.h"

#include <map>
#include <string>
#include <Looper.h>
#include "Agent.h"
#include "AgentList.h"
#include "BlabberSettings.h"
#include "../ui/TalkView.h"
#include "UserID.h"
#include "XMLReader.h"

enum {
	kAuthenticationFailed = 'Aerr'
};

class JabberSpeak : public BHandler, public XMLReader,
	public gloox::ConnectionListener, public gloox::IqHandler
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
	void                     SendConnect(string username = "", string password = "", string realname = "", bool suppress_auto_connect = false);
	void                     SendDisconnect();
	void                     SendSubscriptionRequest(string username); 
	void                     SendUnsubscriptionRequest(string username);
	void                     SetFriendlyName(const gloox::JID& who, BString name);
	void                     RemoveFromRoster(const UserID *removed_user);
	void                     SendMessage(const gloox::Message::MessageType type,
								const gloox::JID&, string message,
								string thread_id);
	void                     SendMessage(const gloox::Message::MessageType type,
								string group_room, string message);
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

	// gloox::ConnectionListener
	void					onConnect() final;
	void					onDisconnect(gloox::ConnectionError e) final;
	bool					onTLSConnect(const gloox::CertInfo& info) final;

	// gloox::IqHandler
	bool					handleIq(const gloox::IQ&) final;
	void					handleIqID(const gloox::IQ&, int) final;

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
	void                    _ParseAgentList(XMLEntity *iq_agent_entity);
	void                    _AcceptPresence(string username);
	void                    _RejectPresence(string username);
	
	// pointer to the singleton
	BlabberSettings        *_blabber_settings;
	
	// some information about the current login status
	string                  _curr_realname;
	string                  _curr_login;
	string                  _password;
	
	bool                    _am_logged_in;
	bool                    _reconnecting;

	bool                    _got_some_agent_info;
	
	// communication state information
	IQMap                   _iq_map;

	// CREATORS
	static JabberSpeak      *_instance;

	// threading
	thread_id                _connection_thread_id;

	gloox::Client*			fClient;
	gloox::Registration*	fRegistration;
};

#endif
