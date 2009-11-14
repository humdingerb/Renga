//////////////////////////////////////////////////
// Blabber [JabberSpeak.h]
//     This monolith talks the jabber language!
//////////////////////////////////////////////////

#ifndef JABBER_SPEAK_H
#define JABBER_SPEAK_H

#ifndef __MAP__
	#include <map>
#endif

#ifndef __STRING__
	#include <string>
#endif

#ifndef _LOOPER_H
	#include <Looper.h>
#endif

#ifndef AGENT_H
	#include "Agent.h"
#endif

#ifndef AGENT_LIST_H
	#include "AgentList.h"
#endif

#ifndef BLABBER_SETTINGS_H
	#include "BlabberSettings.h"
#endif

#ifndef PORT_TALKER_H
	#include "PortTalker.h"
#endif

#ifndef TALK_WINDOW_H
	#include "TalkWindow.h"
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

#ifndef XML_READER_H
	#include "XMLReader.h"
#endif

class JabberSpeak : public PortTalker, public BLooper, public XMLReader {
public:
	enum iq_intent {LOGIN, ROSTER, AGENTS, REGISTER, SEND_REGISTER, UNREGISTER, SEND_UNREGISTER, NEW_USER, MESSAGE, CHAT};

	typedef map<string, iq_intent>                  IQMap;
	typedef map<string, iq_intent>::iterator        IQIter;
	typedef map<string, iq_intent>::const_iterator  ConstIQIter;

public:
	// CREATORS
	static JabberSpeak      *Instance();
	                        ~JabberSpeak();

	void                     Reset(bool leave_network_alone = false);
	void                     JabberSpeakReset();
 
	static void              StaticCalledOnDisconnect(void *obj);
	void                     CalledOnDisconnect();

	void                     MessageReceived(BMessage *msg);

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
	void                     SendPresence(string show = "", string status = "");
	void                     SendLastPresence();
	void	                 SendGroupPresence(string _group_room, string _group_username);
	void	                 SendGroupUnvitation(string _group_room, string _group_username);

	void                     _SendUserRegistration(string username, string password, string resource);
	void                     RegisterWithAgent(string agent);
	void	                 UnregisterWithAgent(string agent);
	
	// INCOMING COMMUNICATION
	void		             OnStartTag(XMLEntity *entity);
	void		             OnEndTag(XMLEntity *entity);
	void		             OnEndEntity(XMLEntity *entity);
	void                     OnTag(XMLEntity *entity);

	// SELECTORS
	const string             CurrentRealName() const;
	const string             CurrentLogin() const;

	// SEMAPHORE ON XMLReader
	void                     LockXMLReader();
	void                     UnlockXMLReader();
	
	
	string					GetRealServer();
	int						GetRealPort();
	          
protected:
	// CREATORS
	                         JabberSpeak();
	                    
private:
    static int32            _SpawnConnectionThread(void *obj);
    void                    _ConnectionThread();

	// OUTGOING COMMUNICATION
	void                    _SendAuthentication(string username, string password, string resource);
	void					_ProcessVersionRequest(string req_id, string req_from);
	void                    _SendAgentRequest();
	void                    _SendRosterRequest();

	// INCOMING COMMUNICATION
	void                    _ProcessRegistration(XMLEntity *iq_register_entity);
	void                    _ProcessUnregistration(XMLEntity *iq_register_entity);
	void                    _SendTransportRegistrationInformation(Agent *agent, string key);
	void                    _SendTransportUnregistrationInformation(Agent *agent, string key);
	void                    _ParseRosterList(XMLEntity *iq_roster_entity);
	void                    _ProcessPresence(XMLEntity *entity);
	void                    _ProcessUserPresence(UserID *user, XMLEntity *entity);
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

	// XMLReader lock
	sem_id                   _xml_reader_lock;
};

#endif
