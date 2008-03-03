//////////////////////////////////////////////////
// Blabber [LoginPreferencesView.h]
//     Allows users to edit login-related user
//     preferences.
//////////////////////////////////////////////////

#ifndef LOGIN_PREFERENCES_VIEW_H
#define LOGIN_PREFERENCES_VIEW_H

#ifndef _BOX_H
	#include <interface/Box.h>
#endif

#ifndef _CHECK_BOX_H
	#include <interface/CheckBox.h>
#endif

#ifndef _TEXT_CONTROL_H
	#include <interface/TextControl.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

class LoginPreferencesView : public BView {
public:
	               LoginPreferencesView(BRect frame);
	virtual       ~LoginPreferencesView();
  
	void           AttachedToWindow();
	   
	void           UpdateFile();

private:
	BBox         *_surrounding;
	BCheckBox    *_auto_login;
	BCheckBox    *_focus_on_chat;

	BBox         *_surrounding_options;
	BCheckBox    *_show_timestamp;
	BCheckBox    *_show_all_chat;
	BCheckBox    *_double_click;

	BBox         *_surrounding_groupchat;
	BTextControl *_name;
};

#endif