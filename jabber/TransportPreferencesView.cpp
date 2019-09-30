//////////////////////////////////////////////////
// Blabber [MessagesPreferencesView.cpp]
//////////////////////////////////////////////////

#include "TransportPreferencesView.h"

#include <cstdio>

#include "AgentList.h"
#include "BlabberSettings.h"
#include "JabberSpeak.h"
#include "Messages.h"
#include "../ui/ModalAlertFactory.h"

TransportPreferencesView::TransportPreferencesView(BRect frame)
	: BView (frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW) {
	_current_transport_registered = false;
	_curr_transport               = "";
	_surrounding                  = NULL;
		
	// BUGBUG also part of the outrageous hack
	_register   = NULL;
	_unregister = NULL;
	
	SetViewColor(216, 216, 216, 255);
}

TransportPreferencesView::~TransportPreferencesView() {
}

void TransportPreferencesView::AttachedToWindow() {
	// don't do it twice
	if (_surrounding) {
		return;
	}

	AgentList *agents = AgentList::Instance();

	if (false) {
		return;
	}
	
	BRect rect(Bounds());

	// box frame
	rect.InsetBy(5.0, 5.0);
	_surrounding = new BBox(rect, NULL, B_FOLLOW_ALL);
	_surrounding->SetLabel("External Chat Systems");

	rect = _surrounding->Bounds();
	
	// transport prefs
	rect.InsetBy(25.0, 45.0);
	rect.right  = rect.left + 375.0;
	rect.bottom = rect.top + 18;

	// transport selection (ICQ)
	_agent_entries = new BPopUpMenu("<select a service>");

	if (agents->GetAgentByService("icq")) {
		BMenuItem *icq = new BMenuItem("Mirabilis ICQ", new BMessage(AGENT_MENU_CHANGED_TO_ICQ));
		icq->SetTarget(this);
		_agent_entries->AddItem(icq);
	}

	_agent_list = new BMenuField(rect, "agent_registrations", "Online Service: ", _agent_entries);	
	
	// username/password fields
	rect.OffsetBy(0.0, 35.0);
	
	rect.bottom = rect.top + 18;
	_username = new BTextControl(rect, "username", "Username: ", NULL, NULL, B_FOLLOW_ALL_SIDES);
	_username->SetDivider(_username->Divider() - 3);

	rect.OffsetBy(0.0, 20.0);
	
	rect.bottom = rect.top + 19;
	_password = new BTextControl(rect, "password", "Password: ", NULL, NULL, B_FOLLOW_ALL_SIDES);
	_password->TextView()->HideTyping(true);
	_password->SetDivider(_password->Divider() - 3);

	// register/unregister buttons
	rect.OffsetBy(0.0, 27.0);

	rgb_color note = {0, 0, 0, 255};
	BFont black_9(be_plain_font);
	black_9.SetSize(9.0);

	rect.bottom += 3;
	
	BRect text_rect(rect);
	text_rect.OffsetTo(B_ORIGIN);
	
	_transport_id_info = new BTextView(rect, NULL, text_rect, &black_9, &note, B_FOLLOW_H_CENTER, B_WILL_DRAW);
	_transport_id_info->SetViewColor(216, 216, 216, 255);
	_transport_id_info->MakeEditable(false);
	_transport_id_info->MakeSelectable(false);
	_transport_id_info->SetText("Please enter all usernames and passwords EXACTLY as you'd type them with their respective clients.");

	rect.OffsetBy(0.0, 33.0);

	rect.bottom -= 3;

	_register = new BButton(rect, "register", "Register", new BMessage(REGISTER_TRANSPORT));	
	_register->SetTarget(this);
	_register->SetEnabled(false);
	
	rect.OffsetBy(0.0, 23.0);

	_unregister = new BButton(rect, "register", "UnRegister", new BMessage(UNREGISTER_TRANSPORT));	
	_unregister->SetTarget(this);
	_unregister->SetEnabled(false);

	rect.OffsetBy(0.0, 40.0);
	rect.bottom = rect.top + 50.0;
		
	text_rect = rect;
	
	text_rect.OffsetTo(B_ORIGIN);
	
/*
	// text run array
	rgb_color blue = {0, 0, 255, 255};
	rgb_color red = {255, 0, 0, 255};

	text_run one = {250, be_plain_font, blue};
	text_run two = {251, be_plain_font, red};

//	text_run one_two[2] = {one, two};
	
	text_run_array be = {1, {one}};
*/
	BTextView *enter_note = new BTextView(rect, NULL, text_rect, &black_9, &note, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	enter_note->SetViewColor(216, 216, 216, 255);
	enter_note->MakeEditable(false);
	enter_note->MakeSelectable(false);
	enter_note->SetText("Note: Transports serve as the means by which XMPP "
		"communicates with external chat systems such as ICQ.  They "
		"are add-on components to the XMPP server you are logged on to and "
		"thus can be ERRATIC and/or BUGGY.  If you are having trouble with a "
		"transport, it is likely the server's fault and not Renga.");

	_surrounding->AddChild(enter_note);
	_surrounding->AddChild(_agent_list);
	_surrounding->AddChild(_transport_id_info);
	_surrounding->AddChild(_username);
	_surrounding->AddChild(_password);
	_surrounding->AddChild(_register);
	_surrounding->AddChild(_unregister);
	AddChild(_surrounding);
}

void TransportPreferencesView::MessageReceived(BMessage *msg) {
	AgentList *agents = AgentList::Instance();
	
	switch (msg->what) {
		case REGISTER_TRANSPORT: {
			if (agents->GetAgentByService(_curr_transport)) {
				if (strlen(_username->Text()) == 0) {
					ModalAlertFactory::Alert("You must enter a username.", "Doh!");
					break;
				}

				if (strlen(_password->Text()) == 0) {
					ModalAlertFactory::Alert("You must enter a password.", "Doh!");
					break;
				}

				// do processing
				Agent *agent = const_cast<Agent *>(agents->GetAgentByService(_curr_transport));

				agent->SetUsername(_username->Text());
				agent->SetPassword(_password->Text());

				JabberSpeak::Instance()->RegisterWithAgent(_curr_transport);
			}
									
			break;
		}

		case UNREGISTER_TRANSPORT: {
			if (agents->GetAgentByService(_curr_transport)) {
				JabberSpeak::Instance()->UnregisterWithAgent(_curr_transport);
			}

			break;
		}

		case TRANSPORT_UPDATE: {
			Agent *agent = const_cast<Agent *>(agents->GetAgentByService(_curr_transport));

			if (agent) {
				_username->SetText(agent->Username().c_str());
				_password->SetText(agent->Password().c_str());
			}

			break;
		}
		
		case AGENT_MENU_CHANGED_TO_ICQ: {
			_curr_transport = "icq";

			if (agents->GetAgentByService(_curr_transport)) {
				_username->SetLabel("ICQ #:");
				_username->SetText(agents->GetAgentByService(_curr_transport)->Username().c_str());
				_password->SetText(agents->GetAgentByService(_curr_transport)->Password().c_str());
			}

			break;
		}
	}

	// buttons enabled
	if (agents->GetAgentByService(_curr_transport)) {
		_current_transport_registered = agents->GetAgentByService(_curr_transport)->IsRegistered();
	}
	
	// BUGBUG workaround for an outrageous hack! ;)
	if (_register && _unregister) {
		if (_current_transport_registered) {
			_register->SetEnabled(false);
			_unregister->SetEnabled(true);
		} else {
			_register->SetEnabled(true);
			_unregister->SetEnabled(false);
		}
	}
}

void TransportPreferencesView::UpdateFile() {
}
