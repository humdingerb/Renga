//////////////////////////////////////////////////
// Blabber [SendTalkWindow.cpp]
//////////////////////////////////////////////////

#include "CustomStatusWindow.h"

#include <cstdio>

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include "support/AppLocation.h"

#include "ui/PictureView.h"
#include "ui/MainWindow.h"

#include "BlabberSettings.h"
#include "GenericFunctions.h"
#include "JabberSpeak.h"
#include "Messages.h"

CustomStatusWindow *CustomStatusWindow::_instance = NULL;

CustomStatusWindow *CustomStatusWindow::Instance() {
	if (_instance == NULL) {
		float main_window_width  = 410;
		float main_window_height = 100;

		BRect frame(GenericFunctions::CenteredFrame(main_window_width, main_window_height));

		_instance = new CustomStatusWindow(frame);
	}

	return _instance;
}


CustomStatusWindow::CustomStatusWindow(BRect frame)
	: BWindow(frame, "Create a Custom Status",
		B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS |
		B_NOT_ZOOMABLE |
		B_NOT_RESIZABLE)
	{

	_chat = new BRadioButton("status", "Chat", NULL);

	_away = new BRadioButton("status", "Away", NULL);

	_xa = new BRadioButton("status", "Extended Away", NULL);

	_dnd = new BRadioButton("status", "Do Not Disturb", NULL);


	BStringView *query = new BStringView(NULL, "Please provide your detailed status:");

	// handle
	_handle = new BTextControl(NULL, NULL, "", NULL);
	_handle->SetDivider(0);

	if (BlabberSettings::Instance()->Data("last-custom-more-exact-status")) {
		_handle->SetText(BlabberSettings::Instance()->Data("last-custom-more-exact-status"));
	} else {
		_handle->SetText("I'm at my computer.");
	}

	BButton *cancel = new BButton("cancel", "Nevermind", new BMessage(JAB_CANCEL));
	cancel->SetTarget(this);

	BButton *ok = new BButton("ok", "OK", new BMessage(JAB_OK));

	ok->MakeDefault(true);
	ok->SetTarget(this);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	BLayoutBuilder::Group<>(this)
		.SetInsets(B_USE_WINDOW_SPACING)
		.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING, 0)
			.Add(_chat)
			.Add(_away)
			.Add(_xa)
			.Add(_dnd)
		.End()
		.AddGroup(B_VERTICAL, B_USE_ITEM_SPACING, 2)
			.Add(query)
			.Add(_handle)
				.AddGroup(B_HORIZONTAL, B_USE_ITEM_SPACING, 0)
				.AddGlue()
				.Add(cancel)
				.Add(ok)
			.End()
		.End()
	.End();

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
				JabberSpeak::Instance()->SendPresence(gloox::Presence::Chat, _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "chat");
			} else if (_away->Value()) {
				JabberSpeak::Instance()->SendPresence(gloox::Presence::Away, _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "away");
			} else if (_xa->Value()) {
				JabberSpeak::Instance()->SendPresence(gloox::Presence::XA, _handle->Text());
				BlabberSettings::Instance()->SetData("last-custom-exact-status", "xa");
			} else if (_dnd->Value()) {
				JabberSpeak::Instance()->SendPresence(gloox::Presence::DND, _handle->Text());
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
