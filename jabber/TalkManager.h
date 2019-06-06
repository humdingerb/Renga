//////////////////////////////////////////////////
// Blabber [TalkManager.h]
//     Handles the simultaneous talk sessions
//     going on.
//////////////////////////////////////////////////

#ifndef TALK_MANAGER_H
#define TALK_MANAGER_H

#include <gloox/messagehandler.h>
#include <gloox/message.h>

#include <map>
#include <string>

#include "GenericFunctions.h"
#include "TalkWindow.h"
#include "XMLEntity.h"

// FIXME could we replace this with MessageSessionHandler?
class TalkManager : public gloox::MessageHandler {
public:
	typedef  std::map<std::string, TalkWindow *>                   TalkMap;
	typedef  std::map<std::string, TalkWindow *>::iterator         TalkIter;
	typedef  std::map<std::string, TalkWindow *>::const_iterator   ConstTalkIter;

	enum     rotation   {ROTATE_FORWARD, ROTATE_BACKWARD};
		
public:
	static TalkManager  *Instance();
      	                ~TalkManager();

	TalkWindow          *CreateTalkSession(const TalkWindow::talk_type type, const UserID *user,
				std::string group_room, std::string group_username, 
				std::string thread = GenericFunctions::GenerateUniqueID(), bool sound_on_new = false);
	void                 handleMessage (const gloox::Message &msg, gloox::MessageSession *session) final;

	std::string          IsExistingWindowToUser(TalkWindow::talk_type type, std::string username);
	std::string          IsExistingWindowToGroup(TalkWindow::talk_type type, std::string group_room);
	void                 UpdateWindowTitles(const UserID *user);
	void                 RemoveWindow(std::string thread_id);

	void                 RotateToNextWindow(TalkWindow *current, rotation direction);
	
	void                 Reset();
	
protected:
 	                     TalkManager();

private:
	static TalkManager *_instance;

	TalkMap             _talk_map;
};

#endif

