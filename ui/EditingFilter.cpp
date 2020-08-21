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

	// if we have modifiers but none are the Alt key
	if (message->FindInt32("modifiers", &modifiers)) {
		return B_DISPATCH_MESSAGE;
	}

	// if the Alt key jives with the command_enter status
	if ((modifiers & B_COMMAND_KEY) != 0 && byte == B_UP_ARROW) {
			_window->RevealPreviousHistory();
	} else if ((modifiers & B_COMMAND_KEY) != 0 && byte == B_DOWN_ARROW) {
			_window->RevealNextHistory();
	} else if (_window->NewlinesAllowed() && (modifiers & B_COMMAND_KEY) == 0 && byte == B_ENTER) {
		_view->Insert("\n");

		return B_SKIP_MESSAGE;
	} else if (!_window->NewlinesAllowed() && (modifiers & B_COMMAND_KEY) != 0 && byte == B_ENTER) {
		_view->Insert("\n");

		return B_SKIP_MESSAGE;
	} else if (_window->NewlinesAllowed() && (modifiers & B_COMMAND_KEY) != 0 && byte == B_ENTER) {
		puts("Go!");
		BMessenger(_window).SendMessage(JAB_CHAT_SENT);
	}
	
	return B_DISPATCH_MESSAGE;
}

