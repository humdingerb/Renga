//////////////////////////////////////////////////
// Blabber [EditingFilter.cpp]
//////////////////////////////////////////////////

#include <interface/InterfaceDefs.h>
#include <app/Message.h>

#include "jabber/Messages.h"

#include "ui/EditingFilter.h"
#include "ui/TalkView.h"

EditingFilter::EditingFilter(BTextView *view, TalkView *window)
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN, NULL) {
	_view   = view;
	_window = window;
}

filter_result EditingFilter::Filter(BMessage *message, __attribute__((unused)) BHandler **target) {
	int32 modifiers;
	int8 byte;

	message->FindInt8("byte", &byte);
	message->FindInt32("modifiers", &modifiers);

	// if the Alt key jives with the command_enter status
	if ((modifiers & B_COMMAND_KEY) != 0 && byte == B_UP_ARROW) {
		_window->RevealPreviousHistory();
		return B_DISPATCH_MESSAGE;
	}

	if ((modifiers & B_COMMAND_KEY) != 0 && byte == B_DOWN_ARROW) {
		_window->RevealNextHistory();
		return B_DISPATCH_MESSAGE;
	}

	if (byte != B_ENTER)
		return B_DISPATCH_MESSAGE;

	if (modifiers & (B_SHIFT_KEY | B_COMMAND_KEY | B_CONTROL_KEY)) {
		_view->Insert("\n");
		return B_SKIP_MESSAGE;
	} else {
		BMessenger(_window).SendMessage(JAB_CHAT_SENT);
		return B_DISPATCH_MESSAGE;
	}
}

