//////////////////////////////////////////////////
// Blabber [BuddyWindow.cpp]
//////////////////////////////////////////////////

#include "BuddyWindow.h"

#include <Application.h>

#include "Agent.h"
#include "AgentList.h"
#include "../support/AppLocation.h"
#include "BlabberSettings.h"
#include "GenericFunctions.h"
#include "Messages.h"
#include "../ui/ModalAlertFactory.h"
#include "JRoster.h"
#include "../ui/PictureView.h"

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

BuddyWindow::BuddyWindow(BRect frame)
	: BWindow(frame, "Adding New Buddy", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {

	BRect rect;

	// encompassing view
	rect = Bounds();
	rect.OffsetTo(B_ORIGIN);
	
	_full_view = new BView(rect, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	_full_view->SetViewColor(216, 216, 216, 255);
	
	// lightbulb
	PictureView *picture = new PictureView("bulb-big");

	rect.left += 140.0;
	rect.InsetBy(6.0, 6.0);
	
	// surrounding box
	_surrounding = new BBox(rect, NULL);
	_surrounding->SetLabel("New Buddy Information");
	
	rect.OffsetTo(B_ORIGIN);
	rect.InsetBy(10.0, 12.0);
	
	// realname
	rect.bottom = rect.top + 18;
	_realname = new BTextControl(rect, "realname", "Nickname: ", NULL, NULL, B_FOLLOW_ALL_SIDES);
	_realname->SetDivider(_realname->Divider() - 35);

	// chat service
	rect.OffsetBy(0.0, 21.0);
	_chat_services_selection = new BPopUpMenu("Jabber");
	_chat_services = new BMenuField(rect, "chat_services", "Online Service: ", _chat_services_selection);	
	_chat_services->SetDivider(_chat_services->Divider() - 33);
	_chat_services_selection->AddItem(new BMenuItem("ICQ", new BMessage(AGENT_MENU_CHANGED_TO_ICQ)));
	_chat_services_selection->AddItem(new BMenuItem("Jabber", new BMessage(AGENT_MENU_CHANGED_TO_JABBER)));

	rect.OffsetBy(0.0, 23.0);
	_handle = new BTextControl(rect, "handle", "Jabber ID: ", NULL, NULL, B_FOLLOW_ALL_SIDES);
	_handle->SetDivider(_handle->Divider() - 35);

	rect.OffsetBy(0.0, 23.0);
	rect.left = _surrounding->Bounds().left + 12.0;
	rect.right = _surrounding->Bounds().right - 12.0;
	rect.bottom += 19.0;
	
	rgb_color note = {0, 0, 0, 255};
	BFont black_9(be_plain_font);
	black_9.SetSize(9.0);

	BRect text_rect(rect);
	text_rect.OffsetTo(B_ORIGIN);
	
	_enter_note = new BTextView(rect, NULL, text_rect, &black_9, &note, B_FOLLOW_H_CENTER, B_WILL_DRAW);
	_enter_note->SetViewColor(216, 216, 216, 255);
	_enter_note->MakeEditable(false);
	_enter_note->MakeSelectable(false);
	_enter_note->SetText("Please enter a Jabber ID of the form username@server (e.g., beoslover@jabber.org).");

	// cancel button
	rect.OffsetBy(102.0, 41.0);
	rect.right = rect.left + 65;
	rect.bottom = rect.top + 18;

	_cancel = new BButton(rect, "cancel", "Nevermind", new BMessage(JAB_CANCEL));
	_cancel->SetTarget(this);

	// ok button
	rect.OffsetBy(75.0, 0.0);
	rect.right = rect.left + 92;

	_ok = new BButton(rect, "ok", "", new BMessage(JAB_OK));
	_ok->MakeDefault(true);
	_ok->SetTarget(this);

	// fill in defaults?
	_chat_services_selection->FindItem("Jabber")->SetMarked(true);

	_ok->SetLabel("Add My Buddy");

	// add GUI components to BView
	_full_view->AddChild(picture);
	_surrounding->AddChild(_realname);
	_surrounding->AddChild(_enter_note);
	_surrounding->AddChild(_chat_services);
	_surrounding->AddChild(_handle);
	_surrounding->AddChild(_cancel);
	_surrounding->AddChild(_ok);
	_full_view->AddChild(_surrounding);
	AddChild(_full_view);

	// focus to start
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

		case AGENT_MENU_CHANGED_TO_JABBER: {
			_enter_note->SetText("Please enter a Jabber ID of the form username@server (e.g., beoslover@jabber.org).");
			_handle->SetLabel("Jabber ID:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_ICQ: {
			_enter_note->SetText("Please enter the user's ICQ numeric ID (e.g., 99818234).");
			_handle->SetLabel("ICQ #:");
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
