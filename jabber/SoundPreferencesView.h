//////////////////////////////////////////////////
// Blabber [SoundPreferencesView.h]
//     Presents user preferences for
//     editing sound event.
//////////////////////////////////////////////////

#ifndef SOUND_PREFERENCES_VIEW_H
#define SOUND_PREFERENCES_VIEW_H

#ifndef __STRING__
	#include <string>
#endif

#ifndef _BOX_H
	#include <interface/Box.h>
#endif

#ifndef _BUTTON_H
	#include <interface/Button.h>
#endif

#ifndef _CHECK_BOX_H
	#include <interface/CheckBox.h>
#endif

#ifndef _MENU_FIELD_H
	#include <interface/MenuField.h>
#endif

#ifndef _MENU_ITEM_H
	#include <interface/MenuItem.h>
#endif

#ifndef _POP_UP_MENU_H
	#include <interface/PopUpMenu.h>
#endif

#ifndef _TEXT_VIEW_H
	#include <interface/TextView.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

#ifndef _FILE_PANEL_H
	#include <storage/FilePanel.h>
#endif

class SoundPreferencesView : public BView {
public:
	               SoundPreferencesView(BRect frame);
	virtual       ~SoundPreferencesView();

	void           AttachedToWindow();
	void           MessageReceived(BMessage *msg);
	
	void           UpdateFile();

private:
	BBox         *_surrounding;
	BBox         *_surrounding_options;

	BMenuField   *_new_chat_field;
	BPopUpMenu   *_new_chat_selection;
	BButton      *_test_new_chat;

	BMenuField   *_message_field;
	BPopUpMenu   *_message_selection;
	BButton      *_test_message;
	
	BMenuField   *_now_online_field;
	BPopUpMenu   *_now_online_selection;
	BButton      *_test_online;

	BMenuField   *_now_offline_field;
	BPopUpMenu   *_now_offline_selection;
	BButton      *_test_offline;

	BMenuField   *_alert_field;
	BPopUpMenu   *_alert_selection;
	BButton      *_test_alert;

	BMenuItem    *_old_new_message_item;
	BMenuItem    *_old_message_item;
	BMenuItem    *_old_user_online_item;
	BMenuItem    *_old_user_offline_item;
	BMenuItem    *_old_alert_item;
	
	BCheckBox    *_groupchat_sounds;
	
	BFilePanel   *_fp;
};

#endif