//////////////////////////////////////////////////
// Blabber [ChangeNameWindow.cpp]
//////////////////////////////////////////////////

#ifndef CHANGE_NAME_WINDOW_H
	#include "ChangeNameWindow.h"
#endif

#include <InterfaceKit.h>

#ifndef __CSTDIO__
	#include <cstdio>
#endif

#ifndef _BUTTON_H
	#include <interface/Button.h>
#endif

#ifndef _STRING_VIEW_H
	#include <interface/StringView.h>
#endif

#ifndef APP_LOCATION_H
	#include "AppLocation.h"
#endif

#ifndef BLABBER_SETTINGS_H
	#include "BlabberSettings.h"
#endif

#ifndef GENERIC_FUNCTIONS_H
	#include "GenericFunctions.h"
#endif

#ifndef JABBER_SPEAK_H
	#include "JabberSpeak.h"
#endif

#ifndef MESSAGES_H
	#include "Messages.h"
#endif

#ifndef MODAL_ALERT_FACTORY_H
	#include "ModalAlertFactory.h"
#endif

#ifndef PICTURE_VIEW_H
	#include "PictureView.h"
#endif

#ifndef TALK_MANAGER_H
	#include "TalkManager.h"
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

ChangeNameWindow::ChangeNameWindow(UserID *changing_user)
	: BWindow(BRect(0, 0, 0, 0), "Changing User Name", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {
	_changing_user = changing_user;
	
	// determine window size
	BRect rect;

	float login_window_width  = 410;
	float login_window_height = 100; 
		
	// create window frame position
	rect = GenericFunctions::CenteredFrame(login_window_width, login_window_height);
	
	// set it
	ResizeTo(rect.Width(), rect.Height());
	MoveTo(rect.LeftTop());
	
	// encompassing view
	rect = Bounds();
	rect.OffsetTo(B_ORIGIN);

	_full_view = new BView(rect, "main-full", B_FOLLOW_ALL, B_WILL_DRAW);
	_full_view->SetViewColor(216, 216, 216, 255);

	rect = Bounds();

	// lightbulb
	PictureView *picture = new PictureView(AppLocation::Instance()->AbsolutePath("resources/graphics/bulb-normal.png").c_str());
	
	// query
	rect.left = 80.0;
	rect.InsetBy(5.0, 5.0);

	_surrounding = new BBox(rect, NULL);
	_surrounding->SetLabel("Change Roster User");
	
	rect.OffsetTo(B_ORIGIN);
	rect.InsetBy(6.0, 12.0);
	rect.bottom = rect.top + 18;
	
	BStringView *query = new BStringView(rect, NULL, "Specify the new \"Nickname\" you'd like to use:");
	
	// handle
	rect.OffsetBy(-2.0, 18.0);
	_handle = new BTextControl(rect, NULL, NULL, "", NULL);
	_handle->SetDivider(0);
	
	if (BlabberSettings::Instance()->Data("last-talk-sent-to")) {
		_handle->SetText(BlabberSettings::Instance()->Data("last-talk-sent-to"));
	} else {
		_handle->SetText("somebody@jabber.org");
	}
	
	// cancel button
	rect.OffsetBy(135.0, 26.0);
	rect.right = rect.left + 65;

	BButton *cancel = new BButton(rect, "cancel", "Nevermind", new BMessage(JAB_CANCEL));
	cancel->SetTarget(this);

	// ok button
	rect.OffsetBy(75.0, 0.0);
	rect.right = rect.left + 92;

	BButton *ok = new BButton(rect, "ok", "Change Name", new BMessage(JAB_OK));

	ok->MakeDefault(true);
	ok->SetTarget(this);
		
	_full_view->AddChild(picture);
	_surrounding->AddChild(query);
	_surrounding->AddChild(_handle);
	_surrounding->AddChild(cancel);
	_surrounding->AddChild(ok);
	_full_view->AddChild(_surrounding);
	
	AddChild(_full_view);

	// focus
	_handle->SetText(_changing_user->FriendlyName().c_str());
	_handle->MakeFocus(true);
}

ChangeNameWindow::~ChangeNameWindow() {
}

void ChangeNameWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		//// JAB_OK
		case JAB_OK: {
			if (!strcmp(_handle->Text(), "")) {
				ModalAlertFactory::Alert("You cannot erase your buddy's name.  If you're trying to remove this buddy, please use the \"Remove Buddy\" item on the user.", "Oops!");
				_handle->MakeFocus(true);

				return;
			}
			
			// change to the new friendly name
			_changing_user->SetFriendlyName(_handle->Text());

			// re-add to roster
			JabberSpeak::Instance()->AddToRoster(_changing_user);

			// update window title
			TalkManager::Instance()->UpdateWindowTitles(_changing_user);

			PostMessage(B_QUIT_REQUESTED);
						
			break;
		}

		//// JAB_CANCEL
		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	}

}
