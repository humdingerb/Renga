//////////////////////////////////////////////////
// Blabber [TalkManager.h]
//     Handles the simultaneous talk sessions
//     going on.
//////////////////////////////////////////////////

#ifndef TALK_MANAGER_H
#define TALK_MANAGER_H

#ifndef __MAP__
	#include <map>
#endif

#ifndef GENERIC_FUNCTIONS_H
	#include "GenericFunctions.h"
#endif

#ifndef TALK_WINDOW_H
	#include "TalkWindow.h"
#endif

#ifndef XML_ENTITY_H
	#include "XMLEntity.h"
#endif

class TalkManager {
public:
	typedef  map<string, TalkWindow *>                   TalkMap;
	typedef  map<string, TalkWindow *>::iterator         TalkIter;
	typedef  map<string, TalkWindow *>::const_iterator   ConstTalkIter;

	enum     rotation   {ROTATE_FORWARD, ROTATE_BACKWARD};
		
public:
	static TalkManager  *Instance();
      	                ~TalkManager();

	TalkWindow          *CreateTalkSession(const TalkWindow::talk_type type, const UserID *user, string group_room, string group_username, string thread = GenericFunctions::GenerateUniqueID(), bool sound_on_new = false);
	void                 ProcessMessageData(XMLEntity *entity);	

	string               IsExistingWindowToUser(TalkWindow::talk_type type, string username);
	string               IsExistingWindowToGroup(TalkWindow::talk_type type, string group_room);
	void                 UpdateWindowTitles(const UserID *user);
	void                 RemoveWindow(string thread_id);

	void                 RotateToNextWindow(TalkWindow *current, rotation direction);
	
	void                 Reset();
	
protected:
 	                     TalkManager();

private:
	static TalkManager *_instance;

	TalkMap             _talk_map;
};

#endif