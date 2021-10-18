//////////////////////////////////////////////////
// Blabber [BuddyWindow.cpp]
//////////////////////////////////////////////////

#include "BuddyWindow.h"

#include "support/AppLocation.h"

#include "ui/ModalAlertFactory.h"
#include "ui/PictureView.h"

#include "Agent.h"
#include "AgentList.h"
#include "BlabberSettings.h"
#include "GenericFunctions.h"
#include "Messages.h"
#include "JRoster.h"

#include <Application.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include <cstdio>
#include <string.h>

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


static const char* sXMPPHelpText = "Please enter a JID of the form username@server (e.g., beoslover@jabber.org).";


BuddyWindow::BuddyWindow(BRect frame)
	: BWindow(frame, "Add Contact", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE)
{
	_realname = new BTextControl("realname", "Nickname:", "", NULL);

	// FIXME use a BOptionPopUp instead
	_chat_services_selection = new BPopUpMenu("Jabber");
	_chat_services = new BMenuField("chat_services", "Online Service: ",
		_chat_services_selection);
	_chat_services_selection->AddItem(new BMenuItem("IRC",
		new BMessage(AGENT_MENU_CHANGED_TO_IRC)));
	_chat_services_selection->AddItem(new BMenuItem("XMPP",
		new BMessage(AGENT_MENU_CHANGED_TO_JABBER)));
	_chat_services_selection->FindItem("XMPP")->SetMarked(true);

	_handle = new BTextControl("handle", "Jabber ID:", "", NULL);

	_enter_note = new BStringView("enter", sXMPPHelpText);
	_enter_note->SetExplicitMinSize(BSize(_enter_note->StringWidth(sXMPPHelpText) + 24,
		B_SIZE_UNSET));

	_cancel = new BButton("Cancel", new BMessage(JAB_CANCEL));
	_cancel->SetTarget(this);

	_ok = new BButton("ok", new BMessage(JAB_OK));
	_ok->MakeDefault(true);
	_ok->SetTarget(this);

	_ok->SetLabel("Add Contact");

	// add GUI components to BView
	SetLayout(new BGridLayout());
	BLayoutBuilder::Grid<>(this)
		.SetInsets(B_USE_WINDOW_SPACING)
		.AddTextControl(_realname, 0, 0)
		.AddMenuField(_chat_services, 0, 1)
		.AddTextControl(_handle, 0, 2)
		.Add(_enter_note, 0, 3, 2, 1)
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING, 0, 4, 2, 1)
			.AddGlue()
			.Add(_cancel)
			.Add(_ok)
		.End()
	.End();

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

		case AGENT_MENU_CHANGED_TO_JABBER:
		{
			_enter_note->SetText(sXMPPHelpText);
			_handle->SetLabel("Jabber ID:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_IRC:
		{
			_enter_note->SetText("Please enter the user's IRC nickname and server (e.g., haikulover%irc.oftc.net).");
			_handle->SetLabel("IRC handle:");
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
	string username = _handle->Text();

	// if not Jabber
	if (strcasecmp(_handle->Label(), "Jabber ID:")) {
		username = GenericFunctions::CrushOutWhitespace(username);
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
