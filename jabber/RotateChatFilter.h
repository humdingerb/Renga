//////////////////////////////////////////////////
// Blabber [RotateChatFilter.h]
//////////////////////////////////////////////////

#ifndef ROTATE_CHAT_FILTER_H
#define ROTATE_CHAT_FILTER_H

#ifndef _MESSAGE_FILTER_H
	#include <MessageFilter.h>
#endif

#ifndef TALK_MANAGER_H
	#include "TalkManager.h"
#endif

#ifndef TALK_WINDOW_H
	#include "TalkWindow.h"
#endif

class RotateChatFilter : public BMessageFilter {
public:
	                      RotateChatFilter(TalkWindow *window);

	virtual filter_result Filter(BMessage *message, BHandler **target);

private:
	TalkWindow           *_window;
};

#endif

