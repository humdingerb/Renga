//////////////////////////////////////////////////
// Blabber [BuddyWindow.h]
//     Lets a user add or edit a roster buddy.
//////////////////////////////////////////////////

#ifndef BUDDY_WINDOW_H
#define BUDDY_WINDOW_H

#ifndef _BOX_H
	#include <interface/Box.h>
#endif

#ifndef _BUTTON_H
	#include <interface/Button.h>
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

#ifndef _TEXT_CONTROL_H
	#include <interface/TextControl.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

class BuddyWindow : public BWindow {
public:
	static BuddyWindow  *Instance();
	                    ~BuddyWindow();

	void	             MessageReceived(BMessage *msg);
	bool                 QuitRequested();

	void                 AddNewUser();
	
protected:
	                     BuddyWindow(BRect frame);
				
private:
	static BuddyWindow *_instance;
	
	BBox               *_surrounding;
	
	BView              *_full_view;
	BTextControl       *_realname;
	BMenuField         *_chat_services;
	BPopUpMenu         *_chat_services_selection;
	BTextControl       *_handle;
	BTextView          *_enter_note;
	
	BButton            *_cancel;
	BButton            *_ok;
	
};

#endif