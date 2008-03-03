//////////////////////////////////////////////////
// Blabber [ChangeNameWindow.h]
//     Change a roster user's friendly name.
//     (Overwrite it)
//////////////////////////////////////////////////

#ifndef CHANGE_NAME_WINDOW_H
#define CHANGE_NAME_WINDOW_H

#include <InterfaceKit.h>

#ifndef _TEXT_CONTROL_H
	#include <interface/TextControl.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

class ChangeNameWindow : public BWindow {
public:
	                       ChangeNameWindow(UserID *changing_user);
	                      ~ChangeNameWindow();

	void                   MessageReceived(BMessage *msg);
	
private:
	BBox                 *_surrounding;
	UserID               *_changing_user;
	BTextControl         *_handle;
	BView                *_full_view;
};

#endif
