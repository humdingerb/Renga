//////////////////////////////////////////////////
// Blabber [SendTalkWindow.cpp]
//////////////////////////////////////////////////

#include "SendTalkWindow.h"

#include <cstdio>

#include <Button.h>
#include <StringView.h>

#include "Agent.h"
#include "AgentList.h"
#include "../support/AppLocation.h"
#include "BlabberSettings.h"
#include "GenericFunctions.h"
#include "JabberSpeak.h"
#include "Messages.h"
#include "../ui/ModalAlertFactory.h"
#include "../ui/PictureView.h"
#include "TalkManager.h"
#include "UserID.h"

SendTalkWindow::SendTalkWindow(gloox::Message::MessageType type)
	: BWindow(BRect(0, 0, 0, 0), NULL, B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {
	_type = type;
	
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
	if (_type == gloox::Message::Groupchat) {
		_surrounding->SetLabel("Select Group Chat Properties");
	} else {
		_surrounding->SetLabel("Select a User");
	}
	
	rect.OffsetTo(B_ORIGIN);
	rect.InsetBy(6.0, 12.0);
	rect.bottom = rect.top + 18;

	// chat service
	_chat_services_selection = new BPopUpMenu("Jabber");
	_chat_services = new BMenuField(rect, "chat_services", "Online Service: ", _chat_services_selection);	
	_chat_services->SetDivider(_chat_services->Divider() - 33);
	_chat_services_selection->AddItem(new BMenuItem("ICQ", new BMessage(AGENT_MENU_CHANGED_TO_ICQ)));
	BMenuItem *default_item = new BMenuItem("Jabber", new BMessage(AGENT_MENU_CHANGED_TO_JABBER));
	_chat_services_selection->AddItem(default_item);
	default_item->SetMarked(true);

	if (_type == gloox::Message::Groupchat) {
		rect.OffsetBy(0.0, 1.0);
	}
	
	_name = new BTextControl(rect, "name", "Username: ", NULL, NULL, B_FOLLOW_ALL_SIDES);

	rect.OffsetBy(0.0, 24.0);

	if (_type == gloox::Message::Groupchat) {
		rect.OffsetBy(0.0, -1.0);
	}

	if (_type == gloox::Message::Groupchat) {
		_handle = new BTextControl(rect, "handle", "Room Name: ", NULL, NULL, B_FOLLOW_ALL_SIDES);
	} else {
		_handle = new BTextControl(rect, "handle", "Jabber ID: ", NULL, NULL, B_FOLLOW_ALL_SIDES);
	}

	_name->SetDivider(_handle->Divider() - 35);
	_handle->SetDivider(_handle->Divider() - 35);

	if (_type == gloox::Message::Groupchat) {
		if (BlabberSettings::Instance()->Data("last-group-username")) {
			_name->SetText(BlabberSettings::Instance()->Data("last-group-username"));
		} else {
			_name->SetText(BlabberSettings::Instance()->Data("channel-name"));
		}
	}

	if (_type == gloox::Message::Groupchat) {
		if (BlabberSettings::Instance()->Data("last-group-joined")) {
			_handle->SetText(BlabberSettings::Instance()->Data("last-group-joined"));
		} else {
			_handle->SetText("be-speak@jabber.org");
		}
	} else {
		if (BlabberSettings::Instance()->Data("last-talk-sent-to")) {
			_handle->SetText(BlabberSettings::Instance()->Data("last-talk-sent-to"));
		} else {
			_handle->SetText("somebody@jabber.org");
		}
	}
	
	// cancel button
	rect.OffsetBy(135.0, 24.0);
	rect.right = rect.left + 65;

	BButton *cancel = new BButton(rect, "cancel", "Nevermind", new BMessage(JAB_CANCEL));
	cancel->SetTarget(this);

	// ok button
	rect.OffsetBy(75.0, 0.0);
	rect.right = rect.left + 92;

	BButton *ok = new BButton(rect, "ok", "", new BMessage(JAB_OK));

	if (_type == gloox::Message::Normal) {
		SetTitle("Send New Message");
		ok->SetLabel("Send Message");
	} else if (_type == gloox::Message::Chat) {
		SetTitle("Start New Chat");
		ok->SetLabel("Start Chat");
	} else if (_type == gloox::Message::Groupchat) {
		SetTitle("Start New Group Chat");
		ok->SetLabel("Start Chat");
	}

	ok->MakeDefault(true);
	ok->SetTarget(this);
		
	_full_view->AddChild(picture);

	if (_type == gloox::Message::Groupchat) {
		_surrounding->AddChild(_name);
	} else {
		_surrounding->AddChild(_chat_services);
	}
	
	_surrounding->AddChild(_handle);
	_surrounding->AddChild(cancel);
	_surrounding->AddChild(ok);
	_full_view->AddChild(_surrounding);
	
	AddChild(_full_view);

	// focus
	_handle->MakeFocus(true);
}

SendTalkWindow::~SendTalkWindow() {
}

void SendTalkWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case AGENT_MENU_CHANGED_TO_JABBER: {
//			_enter_note->SetText("Please enter a Jabber ID of the form username@server (e.g., beoslover@jabber.org).");
			_handle->SetLabel("Jabber ID:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_ICQ: {
//			_enter_note->SetText("Please enter the user's ICQ numeric ID (e.g., 99818234).");
			_handle->SetLabel("ICQ #:");
			break;
		}

		//// JAB_OK
		case JAB_OK: {
			if (_type == gloox::Message::Groupchat) {
				if (!ValidateGroupRoom()) {
					break;
				}
				
				BlabberSettings::Instance()->SetData("last-group-joined", _handle->Text());
				BlabberSettings::Instance()->SetData("last-group-username", _name->Text());

				// create session
				TalkManager::Instance()->CreateTalkSession(_type, NULL, _handle->Text(), _name->Text(), NULL);

				// Add to bookmarks - TODO let user decide if they want autojoin?
				BookmarkManager::Instance().SetBookmark(_handle->Text(), _name->Text(), "", true);

				PostMessage(B_QUIT_REQUESTED);
			} else {
				string user(ValidateUser());

				if (!user.empty()) {
					BlabberSettings::Instance()->SetData("last-talk-sent-to", user.c_str());

					// create session
					gloox::JID jid(user);
					TalkManager::Instance()->CreateTalkSession(_type, &jid, "", "", NULL);
				}

				PostMessage(B_QUIT_REQUESTED);
			}			
				
			break;
		}

		//// JAB_CANCEL
		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	}
}

bool SendTalkWindow::ValidateGroupRoom() {
	char buffer[4096];

	if (!strcmp(_handle->Text(), "")) {
		sprintf(buffer, "Please specify the group's roomname."); 
		ModalAlertFactory::Alert(buffer, "Oops!");
		_handle->MakeFocus(true);

		return false;
	}
	
	if (!strcmp(_name->Text(), "")) {
		sprintf(buffer, "Please specify your handle."); 
		ModalAlertFactory::Alert(buffer, "Oops!");
		_handle->MakeFocus(true);

		return false;
	}
	
	return true;
}

string SendTalkWindow::ValidateUser() {
	char buffer[4096];

	if (!strcmp(_handle->Text(), "")) {
		sprintf(buffer, "Please specify a user's handle."); 
		ModalAlertFactory::Alert(buffer, "Oops!");
		_handle->MakeFocus(true);

		return "";
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
		
		return "";
	}
	
	return username;
}
