//////////////////////////////////////////////////
// Blabber [SendTalkWindow.h]
//     Send a talk request without need for the
//     user to be on the buddy list.
//////////////////////////////////////////////////

#ifndef SEND_TALK_WINDOW_H
#define SEND_TALK_WINDOW_H

#include <interface/MenuField.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <interface/TextControl.h>
#include <interface/Window.h>
#include "../ui/TalkView.h"

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
