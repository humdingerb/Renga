//////////////////////////////////////////////////
// Blabber [ChangeNameWindow.h]
//     Change a roster user's friendly name.
//     (Overwrite it)
//////////////////////////////////////////////////

#pragma once

#include <TextControl.h>
#include <Window.h>

#include "UserID.h"

class ChangeNameWindow : public BWindow {
public:
	                       ChangeNameWindow(const gloox::JID& changing_user, BString oldName);
	                      ~ChangeNameWindow();

	void                   MessageReceived(BMessage *msg);
	
private:
	const gloox::JID&     _changing_user;
	BTextControl         *_handle;
};
