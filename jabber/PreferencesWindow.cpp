//////////////////////////////////////////////////
// Blabber [PreferencesWindow.cpp]
//////////////////////////////////////////////////

#ifndef PREFERENCES_WINDOW_H
	#include "PreferencesWindow.h"
#endif

#ifndef BLABBER_SETTINGS_H
	#include "BlabberSettings.h"
#endif

#ifndef GENERIC_FUNCTIONS_H
	#include "GenericFunctions.h"
#endif

#ifndef MESSAGE_REPEATER_H
	#include "MessageRepeater.h"
#endif

#ifndef MESSAGES_H
	#include "Messages.h"
#endif

#include <stdlib.h>

PreferencesWindow *PreferencesWindow::_instance = NULL;

PreferencesWindow *PreferencesWindow::Instance() {
	if (_instance == NULL) {
		_instance = new PreferencesWindow();
	}
	
	return _instance;
}

PreferencesWindow::PreferencesWindow()
	: BWindow(BRect(0, 0, 0, 0), "User Preferences", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {

	// add self to message family
	MessageRepeater::Instance()->AddTarget(this);

	// determine window size
	BRect rect;

	float login_window_width  = 450;
	float login_window_height = 400; 
		
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

	// set up tab strip
	rect.InsetBy(10.0, 10.0);
	rect.bottom -= 40.0;

	_tab_strip = new BTabView(rect, NULL);
	
	// set up tabs
	rect = _tab_strip->Bounds();
	
	rect.InsetBy(5.0, 5.0);
	rect.bottom -= _tab_strip->TabHeight();
	
	_login_view = new LoginPreferencesView(rect);
	
	_tab_login = new BTab();
	_tab_strip->AddTab(_login_view, _tab_login);
	_tab_login->SetLabel("Messages/Chat");

	_messages_view = new MessagesPreferencesView(rect);

	_tab_messages = new BTab();
	_tab_strip->AddTab(_messages_view, _tab_messages);
	_tab_messages->SetLabel("Communication");

	_transport_view = new TransportPreferencesView(rect);

	_tab_transport = new BTab();
	_tab_strip->AddTab(_transport_view, _tab_transport);
	_tab_transport->SetLabel("Transports");

	_sounds_view = new SoundPreferencesView(rect);

	_tab_sounds = new BTab();
	_tab_strip->AddTab(_sounds_view, _tab_sounds);
	_tab_sounds->SetLabel("Sounds");

	// cancel button
	rect.bottom = rect.top + 18;
	rect.OffsetBy(235.0, _tab_strip->Bounds().Height() + 20.0);
	rect.right = rect.left + 92;

	_cancel = new BButton(rect, "cancel", "Ignore Changes", new BMessage(JAB_CANCEL), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	_cancel->SetTarget(this);

	// ok button
	rect.OffsetBy(100.0, 0.0);
	rect.right = rect.left + 92;

	_ok = new BButton(rect, "ok", "Save Changes", new BMessage(JAB_OK),  B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	_ok->MakeDefault(true);
	_ok->SetTarget(this);

	_full_view->AddChild(_tab_strip);
	_full_view->AddChild(_cancel);
	_full_view->AddChild(_ok);

	// attach all-encompassing main view to window
	AddChild(_full_view);
}

PreferencesWindow::~PreferencesWindow() {
	// remove self from message family
	MessageRepeater::Instance()->RemoveTarget(this);
	
	_instance = NULL;
}

void PreferencesWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case TRANSPORT_UPDATE: {
			_transport_view->MessageReceived(msg);
			
			break;
		}
		
		case JAB_OK: {
			((MessagesPreferencesView *)(_tab_messages->View()))->UpdateFile();
			((LoginPreferencesView *)(_tab_login->View()))->UpdateFile();
			((SoundPreferencesView *)(_tab_sounds->View()))->UpdateFile();

			// save preferences now
			char *str = BlabberSettings::Instance()->EntityTree()->ToString();

			if (str) {
				BlabberSettings::Instance()->WriteToFile();
				free(str);
			}

			PostMessage(B_QUIT_REQUESTED);

			break;
		}

		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	}
}
			
bool PreferencesWindow::QuitRequested() {
	return true;
}
