//////////////////////////////////////////////////
// Blabber [SendTalkWindow.h]
//     Send a talk request without need for the
//     user to be on the buddy list.
//////////////////////////////////////////////////

#ifndef SEND_TALK_WINDOW_H
#define SEND_TALK_WINDOW_H

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

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

#ifndef TALK_WINDOW_H
	#include "TalkWindow.h"
#endif

#include <string>

class SendTalkWindow : public BWindow {
public:
	                         SendTalkWindow(gloox::Message::MessageType type);
	                        ~SendTalkWindow();

	void                     MessageReceived(BMessage *msg);

	bool                     ValidateGroupRoom();
	std::string              ValidateUser();
	
private: 
	BBox                   *_surrounding;
	gloox::Message::MessageType _type;
	BMenuField             *_chat_services;
	BPopUpMenu             *_chat_services_selection;
	BTextControl           *_handle;
	BTextControl           *_name;
	BView                  *_full_view;
};

#endif
