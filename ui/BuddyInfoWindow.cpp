//////////////////////////////////////////////////
// Blabber [BuddyInfoWindow.cpp]
//////////////////////////////////////////////////

#include "BuddyInfoWindow.h"

#include <Button.h>
#include <GroupView.h>
#include <LayoutBuilder.h>

#include "support/AppLocation.h"

#include "ui/PictureView.h"

#include "../jabber/GenericFunctions.h"
#include "../jabber/Messages.h"

BuddyInfoWindow::BuddyInfoWindow(UserID *querying_user)
	: BWindow(BRect(0, 0, 0, 0), "User Information", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	BGroupView* full_view = new BGroupView(B_VERTICAL);
	full_view->GroupLayout()->SetInsets(B_USE_WINDOW_SPACING);

	// lightbulb
	PictureView *picture = new PictureView("bulb-normal");
	
	BGridView* surrounding = new BGridView();

	// USER INFORMATION
	BStringView *friendly_label, *friendly_name;
	BStringView *jabberid_label, *jabberid_name;
	BStringView *status_label, *status_name;
	BStringView *realstatus_label = NULL, *realstatus_name = NULL;

	friendly_label = new BStringView(NULL, "Nickname:");
	if (querying_user->FriendlyName().empty()) {
		friendly_name  = new BStringView(NULL, "<unknown>");
	} else {
		friendly_name  = new BStringView(NULL, querying_user->FriendlyName().c_str());
	}

	if (!querying_user->MoreExactOnlineStatus().empty()) {
		status_label = new BStringView(NULL, "Personalized Status:");
		status_name  = new BStringView(NULL, querying_user->MoreExactOnlineStatus().c_str());

		surrounding->GridLayout()->AddView(status_label, 0, 1);
		surrounding->GridLayout()->AddView(status_name, 1, 1);
	}

	if (!querying_user->ExactOnlineStatus().empty()) {
		realstatus_label = new BStringView(NULL, "Official Status:");
		
		if (querying_user->ExactOnlineStatus() == "xa") {
			realstatus_name  = new BStringView(NULL, "Extended Away");
		} else if (querying_user->ExactOnlineStatus() == "away") {
			realstatus_name  = new BStringView(NULL, "Away");
		} else if (querying_user->ExactOnlineStatus() == "chat") {
			realstatus_name  = new BStringView(NULL, "Available for Chat");
		} else if (querying_user->ExactOnlineStatus() == "dnd") {
			realstatus_name  = new BStringView(NULL, "Do Not Disturb");
		}

		surrounding->GridLayout()->AddView(realstatus_label, 0, 2);
		surrounding->GridLayout()->AddView(realstatus_name, 1, 2);
	}

	if (querying_user->UserType() == UserID::JABBER) {
		jabberid_label = new BStringView(NULL, "Jabber ID:");
		jabberid_name  = new BStringView(NULL, querying_user->Handle().c_str());
	} else {
		if (querying_user->UserType() == UserID::ICQ) {
			jabberid_label = new BStringView(NULL, "ICQ #:");
		} else {
			jabberid_label = new BStringView(NULL, "Jabber ID:");
		}
		
		jabberid_name  = new BStringView(NULL, querying_user->JabberUsername().c_str());
	}

	// OK button
	BButton *ok = new BButton("ok", "OK", new BMessage(B_QUIT_REQUESTED));

	ok->MakeDefault(true);
	ok->SetTarget(this);

	BLayoutBuilder::Group<>(full_view)
		.AddGroup(B_HORIZONTAL)
			.Add(picture)
			.Add(surrounding)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(ok)
		.End()
	.End();

	surrounding->GridLayout()->AddView(friendly_label, 0, 0);
	surrounding->GridLayout()->AddView(friendly_name, 1, 0);
	surrounding->GridLayout()->AddView(jabberid_label, 0, 3);
	surrounding->GridLayout()->AddView(jabberid_name, 1, 3);

	AddChild(full_view);

	CenterOnScreen();
}


BuddyInfoWindow::~BuddyInfoWindow()
{
}
