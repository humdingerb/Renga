//////////////////////////////////////////////////
// Blabber [BuddyWindow.cpp]
//////////////////////////////////////////////////

#include "BuddyWindow.h"

#include <Application.h>

#include "support/AppLocation.h"

#include "ui/ModalAlertFactory.h"
#include "ui/PictureView.h"

#include "Agent.h"
#include "AgentList.h"
#include "BlabberSettings.h"
#include "GenericFunctions.h"
#include "Messages.h"
#include "JRoster.h"

#include <cstdio>
#include <string.h>

#include "GridView.h"
#include "GroupLayout.h"
#include "GroupLayoutBuilder.h"
#include "StringView.h"

BuddyWindow *BuddyWindow::_instance = NULL;

BuddyWindow *BuddyWindow::Instance() {
	if (_instance == NULL) {
		float main_window_width  = 440;
		float main_window_height = 165;

		// create window frame position
		BRect frame(GenericFunctions::CenteredFrame(main_window_width, main_window_height));

		// create window singleton
		_instance = new BuddyWindow(frame);
	}

	return _instance;
}


BuddyWindow::~BuddyWindow() {
	_instance = NULL;
}


BuddyWindow::BuddyWindow(BRect frame)
	: BWindow(frame, "Add Contact", B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE) {

	_realname = new BTextControl("realname", "Nickname:", "", NULL);

	_handle = new BTextControl("handle", "XMPP ID:", "", NULL);

	_enter_note = new BStringView("enter", "Please enter a XMPP ID of the form username@server.tld", B_WILL_DRAW);

	_cancel = new BButton("Cancel", new BMessage(JAB_CANCEL));
	_cancel->SetTarget(this);

	_ok = new BButton("ok", new BMessage(JAB_OK));
	_ok->MakeDefault(true);
	_ok->SetTarget(this);

	_ok->SetLabel("Add Contact");

	// add GUI components to BView
	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 4)
		.Add(_realname)
		.Add(_handle)
		.Add(_enter_note)
			.Add(BGroupLayoutBuilder(B_HORIZONTAL, 2)
			.Add(_ok)
			.Add(_cancel))
		);
	_realname->MakeFocus(true);
}


void BuddyWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case JAB_OK: {
			AddNewUser();
			break;
		}

		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	}
}


bool BuddyWindow::QuitRequested() {
	_instance = NULL;
	return true;
}


void BuddyWindow::AddNewUser() {
	char buffer[4096];

	if (!strcmp(_realname->Text(), "")) {
		ModalAlertFactory::Alert("Please specify your buddy's real name.", "Oops!");
		_realname->MakeFocus(true);

		return;
	}

	if (!strcmp(_handle->Text(), "")) {
		sprintf(buffer, "Please specify %s's %s handle (or screenname).", _realname->Text(), (_chat_services_selection->FindMarked())->Label());
		ModalAlertFactory::Alert(buffer, "Oops!");
		_handle->MakeFocus(true);

		return;
	}

	// internally replace the username with a proper one if necessary (transports)
	Agent *agent;
	string username = _handle->Text();

	// if not Jabber
	if (strcasecmp(_handle->Label(), "Jabber ID:")) {
		username = GenericFunctions::CrushOutWhitespace(username);
	}

	if (!strcasecmp(_chat_services->Menu()->FindMarked()->Label(), "ICQ")) {
		agent = AgentList::Instance()->GetAgentByService("icq");

		if (agent) {
			username += "@";
			username += agent->JID();
		}
	}

	// make a user to validate against
	std::string validate = UserID::WhyNotValidJabberHandle(username);

	if (!strcasecmp(_handle->Label(), "Jabber ID:") && validate.size()) {
		sprintf(buffer, "%s is not a valid Jabber ID for the following reason:\n\n%s\n\nPlease correct it.", _handle->Text(), validate.c_str());
		ModalAlertFactory::Alert(buffer, "Hmm, better check that...");
		_handle->MakeFocus(true);

		return;
	}

	// make sure it's not a duplicate of one already existing (unless itself)
	JRoster::Instance()->Lock();
	if (JRoster::Instance()->FindUser(JRoster::COMPLETE_HANDLE, _handle->Text())) {
		sprintf(buffer, "%s already exists in your buddy list.  Please choose another so you won't get confused.", _handle->Text());
		ModalAlertFactory::Alert(buffer, "Good Idea!");
		_handle->MakeFocus(true);

		JRoster::Instance()->Unlock();
		return;
	}
	JRoster::Instance()->Unlock();

	gloox::JID jid(username);

	// add this user to the roster
	JRoster::Instance()->Lock();
	JRoster::Instance()->AddNewUser(jid, _realname->Text());
	JRoster::Instance()->Unlock();

	// alert all RosterViews
	JRoster::Instance()->RefreshRoster();

	// close window explicitly
	PostMessage(B_QUIT_REQUESTED);
}
