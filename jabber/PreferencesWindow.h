//////////////////////////////////////////////////
// Blabber [PreferencesWindow.h]
//     Makes many configuration file preferences
//     available to the user.
//////////////////////////////////////////////////

#ifndef PREFERENCES_WINDOW_H
#define PREFERENCES_WINDOW_H

#ifndef _BUTTON_H
	#include <interface/Button.h>
#endif

#ifndef LOGIN_PREFERENCES_VIEW_H
	#include "LoginPreferencesView.h"
#endif

#ifndef MESSAGES_PREFERENCES_VIEW_H
	#include "MessagesPreferencesView.h"
#endif

#ifndef SOUND_PREFERENCES_VIEW_H
	#include "SoundPreferencesView.h"
#endif

#ifndef TRANSPORT_PREFERENCES_VIEW_H
	#include "TransportPreferencesView.h"
#endif

#ifndef _TAB_VIEW_H
	#include <interface/TabView.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

class PreferencesWindow : public BWindow {
public:
	static PreferencesWindow  *Instance();
	
public:
	                           PreferencesWindow();
	   	                      ~PreferencesWindow();

	void                       MessageReceived(BMessage *msg);
	bool                       QuitRequested();

private:
	static PreferencesWindow *_instance;
	
private:
	BView                    *_full_view;
 
	BTabView                 *_tab_strip;

	BTab                     *_tab_login;
	BTab                     *_tab_messages;
	BTab                     *_tab_transport;
	BTab                     *_tab_sounds;

	LoginPreferencesView     *_login_view;
	MessagesPreferencesView  *_messages_view;
	SoundPreferencesView     *_sounds_view;
	TransportPreferencesView *_transport_view;

	BButton                  *_ok;
	BButton                  *_cancel;
};

#endif