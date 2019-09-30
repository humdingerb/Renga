//////////////////////////////////////////////////
// Blabber [BuddyInfoWindow.cpp]
//////////////////////////////////////////////////

#include "BuddyInfoWindow.h"

#include <interface/Button.h>

#include "../support/AppLocation.h"
#include "GenericFunctions.h"
#include "Messages.h"
#include "../ui/PictureView.h"

BuddyInfoWindow::BuddyInfoWindow(UserID *querying_user)
	: BWindow(BRect(0, 0, 0, 0), "User Information Summary", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {
	// determine window size
	BRect rect;

	float login_window_width  = 450;
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
	_surrounding->SetLabel("User Information");
	
	rect.OffsetTo(B_ORIGIN);
	rect.InsetBy(6.0, 12.0);
	rect.bottom = rect.top + 18;

	// USER INFORMATION
	BRect label_rect(rect), value_rect(rect);
	BStringView *friendly_label, *friendly_name;
	BStringView *jabberid_label, *jabberid_name;
	BStringView *status_label, *status_name;
	BStringView *realstatus_label = NULL, *realstatus_name = NULL;

	label_rect.right -= (label_rect.Width() / 1.4);
	value_rect.left = label_rect.right + 1.0;
	value_rect.right = rect.right;

	friendly_label = new BStringView(label_rect, NULL, "Nickname:");
	if (querying_user->FriendlyName().empty()) {
		friendly_name  = new BStringView(value_rect, NULL, "<unknown>");
	} else {
		friendly_name  = new BStringView(value_rect, NULL, querying_user->FriendlyName().c_str());
	}

	label_rect.OffsetBy(0.0, 15.0);
	value_rect.OffsetBy(0.0, 15.0);

	if (!querying_user->MoreExactOnlineStatus().empty()) {
		status_label = new BStringView(label_rect, NULL, "Personalized Status:");
		status_name  = new BStringView(value_rect, NULL, querying_user->MoreExactOnlineStatus().c_str());

		_surrounding->AddChild(status_label);
		_surrounding->AddChild(status_name);

		label_rect.OffsetBy(0.0, 15.0);
		value_rect.OffsetBy(0.0, 15.0);
	}

	value_rect.right = 275.0;
	
	if (!querying_user->ExactOnlineStatus().empty()) {
		realstatus_label = new BStringView(label_rect, NULL, "Official Status:");
		
		if (querying_user->ExactOnlineStatus() == "xa") {
			realstatus_name  = new BStringView(value_rect, NULL, "Extended Away");
		} else if (querying_user->ExactOnlineStatus() == "away") {
			realstatus_name  = new BStringView(value_rect, NULL, "Away");
		} else if (querying_user->ExactOnlineStatus() == "chat") {
			realstatus_name  = new BStringView(value_rect, NULL, "Available for Chat");
		} else if (querying_user->ExactOnlineStatus() == "dnd") {
			realstatus_name  = new BStringView(value_rect, NULL, "Do Not Disturb");
		}

		_surrounding->AddChild(realstatus_label);
		_surrounding->AddChild(realstatus_name);

		label_rect.OffsetBy(0.0, 15.0);
		value_rect.OffsetBy(0.0, 15.0);
	}

	if (querying_user->UserType() == UserID::JABBER) {
		jabberid_label = new BStringView(label_rect, NULL, "Jabber ID:");
		jabberid_name  = new BStringView(value_rect, NULL, querying_user->Handle().c_str());
	} else {
		if (querying_user->UserType() == UserID::ICQ) {
			jabberid_label = new BStringView(label_rect, NULL, "ICQ #:");
		} else {
			jabberid_label = new BStringView(label_rect, NULL, "Jabber ID:");
		}
		
		jabberid_name  = new BStringView(value_rect, NULL, querying_user->JabberUsername().c_str());
	}

	label_rect.OffsetBy(0.0, 15.0);
	value_rect.OffsetBy(0.0, 15.0);

	rect.OffsetTo(280.0, 56.0);
	rect.right = rect.left + 65;

	// OK button
	BButton *ok = new BButton(rect, "ok", "OK", new BMessage(JAB_OK));

	ok->MakeDefault(true);
	ok->SetTarget(this);
		
	_full_view->AddChild(picture);
	_surrounding->AddChild(friendly_label);
	_surrounding->AddChild(friendly_name);
	_surrounding->AddChild(jabberid_label);
	_surrounding->AddChild(jabberid_name);
	_surrounding->AddChild(ok);
	_full_view->AddChild(_surrounding);
	
	AddChild(_full_view);
}

BuddyInfoWindow::~BuddyInfoWindow() {
}

void BuddyInfoWindow::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case JAB_OK: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	}
}
