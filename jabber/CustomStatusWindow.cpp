//////////////////////////////////////////////////
// Blabber [SendTalkWindow.cpp]
//////////////////////////////////////////////////

#ifndef CUSTOM_STATUS_WINDOW_H
	#include "CustomStatusWindow.h"
#endif

#include <InterfaceKit.h>

#ifndef __CSTDIO__
	#include <cstdio>
#endif

#ifndef _BOX_H
	#include <interface/Box.h>
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

#ifndef BLABBER_MAIN_WINDOW_H
	#include "BlabberMainWindow.h"
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

#ifndef PICTURE_VIEW_H
	#include "PictureView.h"
#endif

CustomStatusWindow *CustomStatusWindow::_instance = NULL;

CustomStatusWindow *CustomStatusWindow::Instance() {
	if (_instance == NULL) {
		_instance = new CustomStatusWindow();
	}
	
	return _instance;
}


CustomStatusWindow::CustomStatusWindow()
	: BWindow(BRect(0, 0, 0, 0), "Create a Custom Status", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {
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
	PictureView *picture = new PictureView("bulb-normal");
	
	// query
	rect.left = 80.0;
	rect.InsetBy(5.0, 5.0);

	_surrounding = new BBox(rect, NULL);
	_surrounding->SetLabel("Specify your Status");
	
	rect.OffsetTo(B_ORIGIN);
	rect.InsetBy(6.0, 19.0);
	rect.bottom = rect.top + 18;
	rect.right = rect.left + 100.0;
		
	_chat = new BRadioButton(rect, "status", "Chat", NULL);	
	rect.OffsetBy(0.0, 15.0);

	_away = new BRadioButton(rect, "status", "Away", NULL);	
	rect.OffsetBy(0.0, 15.0);

	_xa = new BRadioButton(rect, "status", "Extended Away", NULL);	
	rect.OffsetBy(0.0, 15.0);

	_dnd = new BRadioButton(rect, "status", "Do Not Disturb", NULL);	

	rect.OffsetBy(110.0, -48.0);
	rect.right = rect.left + 200.0;
	
	BStringView *query = new BStringView(rect, NULL, "Please provide your detailed status:");

	// handle
	rect.OffsetBy(-2.0, 18.0);
	_handle = new BTextControl(rect, NULL, NULL, "", NULL);
	_handle->SetDivider(0);
	
	if (BlabberSettings::Instance()->Data("last-custom-more-exact-status")) {
		_handle->SetText(BlabberSettings::Instance()->Data("last-custom-more-exact-status"));
	} else {
		_handle->SetText("I'm at my computer.");
	}
	
	// cancel button
	rect.OffsetBy(53.0, 24.0);
	rect.right = rect.left + 65;

	BButton *cancel = new BButton(rect, "cancel", "Nevermind", new BMessage(JAB_CANCEL));
	cancel->SetTarget(this);

	// ok button
	rect.OffsetBy(75.0, 0.0);

	BButton *ok = new BButton(rect, "ok", "OK", new BMessage(JAB_OK));

	ok->MakeDefault(true);
	ok->SetTarget(this);
		
	_full_view->AddChild(picture);
	_surrounding->AddChild(_chat);
	_surrounding->AddChild(_away);
	_surrounding->AddChild(_xa);
	_surrounding->AddChild(_dnd);
	_surrounding->AddChild(query);
	_surrounding->AddChild(_handle);
	_surrounding->AddChild(cancel);
	_surrounding->AddChild(ok);
	_full_view->AddChild(_surrounding);
	
	AddChild(_full_view);
	
	// set defaults
	string exact_status;
	
	if (BlabberSettings::Instance()->Data("last-custom-exact-status")) {
		// get last status
		exact_status = BlabberSettings::Instance()->Data("last-custom-exact-status");

		// map to radio buttons		
		if (exact_status == "away") {
			_away->SetValue(1);
		} else if (exact_status == "xa") {
			_xa->SetValue(1);
		} else if (exact_status == "dnd") {
			_dnd->SetValue(1);
		} else {
			_chat->SetValue(1);
		}
	} else {
		_chat->SetValue(1);
	}

	// focus
	_handle->MakeFocus(true);
}

CustomStatusWindow::~CustomStatusWindow() {
	_instance = NULL;
}

void CustomStatusWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		//// JAB_OK
		case JAB_OK: {
			if (_chat->Value()) {
				JabberSpeak::Instance()->SendPresence("chat", _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "chat");
			} else if (_away->Value()) {
				JabberSpeak::Instance()->SendPresence("away", _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "away");
			} else if (_xa->Value()) {
				JabberSpeak::Instance()->SendPresence("xa", _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "xa");
			} else if (_dnd->Value()) {
				JabberSpeak::Instance()->SendPresence("dnd", _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "dnd");
			}			
	
			BlabberSettings::Instance()->SetTag("last-used-custom-status", true);
			BlabberSettings::Instance()->SetData("last-custom-more-exact-status", _handle->Text());
			BlabberSettings::Instance()->WriteToFile();
			
			// update menu
			BlabberMainWindow::Instance()->SetCustomStatus(_handle->Text());

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
