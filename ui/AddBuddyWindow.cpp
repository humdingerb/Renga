#include "AddBuddyWindow.h"

#include "support/AppLocation.h"

#include "ui/ModalAlertFactory.h"

#include "jabber/Agent.h"
#include "jabber/AgentList.h"
#include "jabber/BlabberSettings.h"
#include "jabber/GenericFunctions.h"
#include "jabber/JRoster.h"
#include "jabber/Messages.h"

#include <Button.h>
#include <Application.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include <cstdio>
#include <string.h>

const int kXMPPSelected = 0;
const int kIRCSelected = 1;

AddBuddyWindow *AddBuddyWindow::fInstance = NULL;

AddBuddyWindow *AddBuddyWindow::Instance() {
	if (fInstance == NULL) {
		float main_window_width  = 440;
		float main_window_height = 165;

		// create window frame position
		BRect frame(GenericFunctions::CenteredFrame(main_window_width, main_window_height));

		// create window singleton
		fInstance = new AddBuddyWindow(frame);
	}

	return fInstance;
}


AddBuddyWindow::~AddBuddyWindow() {
	fInstance = NULL;
}


static const char* sXMPPHelpText = "Please enter a JID of the form username@server (e.g., beoslover@jabber.org).";
static const char* sIRCHelpText  = "Please enter the user's IRC nickname and server (e.g., haikulover%irc.oftc.net).";

AddBuddyWindow::AddBuddyWindow(BRect rect)
	: BWindow(rect, "Add a buddy", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE)
{
	fNickName = new BTextControl("nickName", "Nickname:", "", NULL);

	fServiceSelection = new BOptionPopUp("Service Selection", "Online service:",
		new BMessage(kOptionChanged));
	fServiceSelection->AddOption("XMPP", kXMPPSelected);
	fServiceSelection->AddOption("IRC", kIRCSelected);

	fHandle = new BTextControl("userID", "XMPP ID:", "", NULL);

	fHelpText = new BStringView("helpText", sXMPPHelpText);
	fHelpText->SetExplicitMinSize(BSize(fHelpText->StringWidth(sXMPPHelpText) + 24,
		B_SIZE_UNSET));

	BButton *kCancelButton = new BButton("Cancel", new BMessage(JAB_CANCEL));
	kCancelButton->SetTarget(this);


	BButton *kOkButton = new BButton("ok", new BMessage(JAB_OK));
	kOkButton->MakeDefault(true);
	kOkButton->SetTarget(this);

	kOkButton->SetLabel("Add buddy");

	// add GUI components to BView
	SetLayout(new BGridLayout());
	BLayoutBuilder::Grid<>(this)
		.SetInsets(B_USE_WINDOW_SPACING)
		.AddTextControl(fNickName, 0, 0)
		.Add(fServiceSelection, 0, 1)
		.AddTextControl(fHandle, 0, 2)
		.Add(fHelpText, 0, 3, 2, 1)
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING, 0, 4, 2, 1)
			.AddGlue()
			.Add(kCancelButton)
			.Add(kOkButton)
		.End()
	.End();

	fNickName->MakeFocus(true);
}


void AddBuddyWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case JAB_OK: {
			AddNewUser();
			break;
		}

		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		case kOptionChanged:
		{
			switch (fServiceSelection->SelectedOption()) {
				case kXMPPSelected: {
					fHelpText->SetText(sXMPPHelpText);
					fHandle->SetLabel("XMPP ID:");
					break;
				}
				case kIRCSelected: {
					fHelpText->SetText(sIRCHelpText);
					fHandle->SetLabel("IRC handle:");
					break;
				}
			}
			break;
		}
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}


bool AddBuddyWindow::QuitRequested() {
	fInstance = NULL;
	return true;
}


void AddBuddyWindow::AddNewUser() {
	char buffer[4096];

	if (fNickName->Text()[0] == 0) {
		fNickName->MarkAsInvalid(true);
		fNickName->MakeFocus(true);

		return;
	} else {
		fNickName->MarkAsInvalid(false);
	}

	if (fHandle->Text()[0] == 0) {
		fHandle->MarkAsInvalid(true);
		fHandle->MakeFocus(true);

		return;
	}

	string username = GenericFunctions::CrushOutWhitespace(fHandle->Text());

	// make a user to validate against
	std::string validate = UserID::WhyNotValidJabberHandle(username);

	if (fServiceSelection->SelectedOption() == kXMPPSelected && validate.size()) {
		sprintf(buffer, "%s is not a valid XMPP ID for the following reason:\n\n%s\n\nPlease correct it.", fHandle->Text(), validate.c_str());
		ModalAlertFactory::Alert(buffer, "Hmm, better check that...");
		fHandle->MakeFocus(true);
		fHandle->MarkAsInvalid(true);

		return;
	}

	// make sure it's not a duplicate of one already existing (unless itself)
	JRoster::Instance()->Lock();
	if (JRoster::Instance()->FindUser(JRoster::COMPLETE_HANDLE, fHandle->Text())) {
		sprintf(buffer, "%s already exists in your buddy list.  Please choose another so you won't get confused.", fHandle->Text());
		ModalAlertFactory::Alert(buffer, "OK");
		fHandle->MakeFocus(true);
		fHandle->MarkAsInvalid(true);

		JRoster::Instance()->Unlock();

		return;
	}
	JRoster::Instance()->Unlock();

	gloox::JID jid(username);

	// add this user to the roster
	JRoster::Instance()->Lock();
	JRoster::Instance()->AddNewUser(jid, fNickName->Text());
	JRoster::Instance()->Unlock();

	// alert all RosterViews
	JRoster::Instance()->RefreshRoster();

	// close window explicitly
	PostMessage(B_QUIT_REQUESTED);
}
