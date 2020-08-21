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

RotateChatFilter::RotateChatFilter()
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN, NULL) {
}

filter_result RotateChatFilter::Filter(BMessage *message, __attribute__((unused)) BHandler **target) {
	int8 byte;
	message->FindInt8("byte", &byte);

	if (byte == B_FUNCTION_KEY) {
		int32 key;

		message->FindInt32("key", &key);
		
		if (key == B_F8_KEY) {
			BlabberMainWindow::Instance()->PostMessage(JAB_ROTATE_CHAT_FORWARD);
			return B_SKIP_MESSAGE;
		}
	}

	return B_DISPATCH_MESSAGE;
}

