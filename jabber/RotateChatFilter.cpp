//////////////////////////////////////////////////
// Blabber [EditingFilter.cpp]
//////////////////////////////////////////////////

#ifndef _INTERFACE_DEFS_H
	#include <interface/InterfaceDefs.h>
#endif

#ifndef _MESSAGE_H
	#include <app/Message.h>
#endif

#ifndef ROTATE_CHAT_FILTER_H
	#include "RotateChatFilter.h"
#endif

#ifndef MESSAGES_H
	#include "Messages.h"
#endif

RotateChatFilter::RotateChatFilter(TalkWindow *window)
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN, NULL) {
	_window = window;
}

filter_result RotateChatFilter::Filter(BMessage *message, __attribute__((unused)) BHandler **target) {
	int8 byte;
	message->FindInt8("byte", &byte);

	// if the Alt key jives with the command_enter status
	if (byte == B_FUNCTION_KEY) {
		int32 key;

		message->FindInt32("key", &key);
		
		if (key == B_F8_KEY) {
			TalkManager::Instance()->RotateToNextWindow(_window, TalkManager::ROTATE_FORWARD);
			return B_SKIP_MESSAGE;
		}
	}

	return B_DISPATCH_MESSAGE;
}

