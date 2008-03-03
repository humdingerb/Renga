//////////////////////////////////////////////////
// Blabber [LoginpreferencesView.h]
//////////////////////////////////////////////////

#ifndef LOGIN_PREFERENCES_VIEW_H
	#include "LoginPreferencesView.h"
#endif

#ifndef BLABBER_SETTINGS_H
	#include "BlabberSettings.h"
#endif

LoginPreferencesView::LoginPreferencesView(BRect frame)
	: BView (frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW) {
	SetViewColor(216, 216, 216, 255);

	_surrounding_options = NULL;
}

LoginPreferencesView::~LoginPreferencesView() {
}

void LoginPreferencesView::AttachedToWindow() {
	if (_surrounding_options) {
		return;
	}
	
	BRect frame(Bounds());

	// box frame
	frame.InsetBy(5.0, 5.0);
	frame.bottom = frame.top + 91.0;	

	_surrounding_options = new BBox(frame, NULL, B_FOLLOW_ALL);
	_surrounding_options->SetLabel("Options");

	frame.OffsetBy(0.0, frame.Height() + 10.0);

	_surrounding = new BBox(frame, NULL, B_FOLLOW_ALL);
	_surrounding->SetLabel("/ Syntax");

	frame.OffsetBy(0.0, frame.Height() + 10.0);

	_surrounding_groupchat = new BBox(frame, NULL, B_FOLLOW_ALL);
	_surrounding_groupchat->SetLabel("Group Chat Options");

	frame = _surrounding->Bounds();
	
	// auto-login
	frame.InsetBy(25.0, 28.0);
	frame.right  = frame.left + 375.0;
	frame.bottom = frame.top + 18;

	_auto_login = new BCheckBox(frame, NULL, "Disallow /alert from interrupting me.", NULL);	
	frame.OffsetBy(0.0, 17.0);

	_focus_on_chat = new BCheckBox(frame, NULL, "Prevent new chat windows from interrupting me.", NULL);	

	_auto_login->SetValue(BlabberSettings::Instance()->Tag("suppress-alert"));
	_focus_on_chat->SetValue(BlabberSettings::Instance()->Tag("suppress-chat-focus"));

	_surrounding->AddChild(_auto_login);
	_surrounding->AddChild(_focus_on_chat);

	frame = _surrounding->Bounds();
	
	// auto-login
	frame.InsetBy(25.0, 22.0);
	frame.right  = frame.left + 375.0;
	frame.bottom = frame.top + 18;

	_show_timestamp = new BCheckBox(frame, NULL, "Attach timestamps to all messages.", NULL);	
	frame.OffsetBy(0.0, 17.0);

	_show_all_chat = new BCheckBox(frame, NULL, "Convert all incoming ICQ-style messages to AOL-style chat.", NULL);	
	frame.OffsetBy(0.0, 17.0);

	_double_click = new BCheckBox(frame, NULL, "Double-clicking on a user starts a chat.", NULL);	

	_show_timestamp->SetValue(BlabberSettings::Instance()->Tag("show-timestamp"));
	_show_all_chat->SetValue(BlabberSettings::Instance()->Tag("convert-messages-to-chat"));
	_double_click->SetValue(BlabberSettings::Instance()->Tag("enable-double-click"));

	frame = _surrounding_groupchat->Bounds();
	
	// channel name
	frame.InsetBy(25.0, 28.0);
	frame.right  = frame.left + 375.0;
	frame.bottom = frame.top + 18;

	_name = new BTextControl(frame, NULL, "Channel Name:", BlabberSettings::Instance()->Data("channel-name"), NULL);

	_surrounding_options->AddChild(_show_timestamp);
	_surrounding_options->AddChild(_show_all_chat);
	_surrounding_options->AddChild(_double_click);

	_surrounding_groupchat->AddChild(_name);

	AddChild(_surrounding);
	AddChild(_surrounding_options);
	AddChild(_surrounding_groupchat);
}

void LoginPreferencesView::UpdateFile() {
	BlabberSettings::Instance()->SetTag("suppress-alert", _auto_login->Value());
	BlabberSettings::Instance()->SetTag("suppress-chat-focus", _focus_on_chat->Value());
	BlabberSettings::Instance()->SetTag("show-timestamp", _show_timestamp->Value());
	BlabberSettings::Instance()->SetTag("convert-messages-to-chat", _show_all_chat->Value());
	BlabberSettings::Instance()->SetTag("enable-double-click", _double_click->Value());
	BlabberSettings::Instance()->SetData("channel-name", _name->Text());
}
