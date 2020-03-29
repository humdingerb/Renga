//////////////////////////////////////////////////
// Blabber [TalkWindow.h]
//     A session with another user.
//////////////////////////////////////////////////

#ifndef TALK_WINDOW_H
#define TALK_WINDOW_H

#include <gloox/message.h>

#include <cstdio>
#include <deque>
#include <string>

#include <interface/Box.h>
#include <interface/Button.h>
#include <interface/CheckBox.h>
#include <interface/ListView.h>
#include <interface/MenuBar.h>
#include <interface/MenuItem.h>
#include <interface/ScrollView.h>
#include <interface/StringView.h>
#include <interface/TextView.h>
#include <interface/Window.h>
#include <storage/FilePanel.h>
	
#include "BetterTextView.h"
#include "ChatTextView.h"
#include "ChatWidget.h"
#include "EditingFilter.h"
#include "StatusView.h"
#include "UserID.h"

class TalkWindow : public BWindow {
public:
	enum                 user_type {MAIN_RECIPIENT, LOCAL, OTHER};

public:
	static float         x_placement_offset;
	static float         y_placement_offset;
	
public:  
						TalkWindow(gloox::Message::MessageType type,
							const gloox::JID *user, std::string group_room,
							std::string group_username, bool follow_focus_rules = false);
						~TalkWindow();

	gloox::Message::MessageType 	Type();

	void                 FrameResized(float width, float height);
	void                 MenusBeginning();
	void                 MessageReceived(BMessage *msg);
	bool                 QuitRequested();
	
	void                 Log(const char *buffer);
	std::string          OurRepresentation();
	void                 AddToTalk(std::string username, std::string message, user_type type);
	void                 NewMessage(std::string new_message);
	void                 NewMessage(std::string username, std::string new_message);
	void                 SetThreadID(std::string id);

	bool                 NewlinesAllowed();

	const UserID        *GetUserID();
	std::string          GetGroupRoom();
	std::string          GetGroupUsername();

	int                  CountHyperlinks(std::string message);
	void                 GenerateHyperlinkText(std::string message, text_run standard, text_run_array **tra);

	void                 AddGroupChatter(std::string username);
	void                 RemoveGroupChatter(std::string username);

	void                 RevealPreviousHistory();		
	void                 RevealNextHistory();		

private:
    //xed: new message window title notification
    void 				WindowActivated(bool active);
    void				NotifyWindowTitle();
    BString				originalWindowTitle;

	const UserID          *_user;
	std::string            _group_room;
	std::string            _group_username;
	UserID::online_status  _current_status;
	gloox::Message::MessageType _type;
	std::string            _thread;
	
	// GUI
	BView              *_full_view;

	StatusView         *_status_view;
	
	BMenuBar           *_menubar;
	BMenu              *_file_menu;
	BMenu              *_edit_menu;
	BMenu              *_talk_menu;
	BMenu              *_help_menu;
	BMenu              *_message_menu;
	BMenuItem          *_record_item;
	BMenuItem          *_preferences_item;
	BMenuItem          *_copy_item;
	BMenuItem          *_cut_item;
	BMenuItem          *_paste_item;
	BMenuItem          *_select_all_item;
	BMenuItem          *_undo_item;
	BMenuItem          *_record_entire_item;
	BMenuItem          *_close_item;
	BMenuItem          *_message_1_item;
	BMenuItem          *_message_2_item;
	BMenuItem          *_message_3_item;
	BMenuItem          *_message_4_item;
	BMenuItem          *_message_5_item;
	BMenuItem          *_message_6_item;
	BMenuItem          *_message_7_item;
	BMenuItem          *_message_8_item;
	BMenuItem          *_message_9_item;
	BMenuItem          *_rotate_chat_forward_item;
	BMenuItem          *_rotate_chat_backward_item;
	BMenuItem          *_rotate_buddy_list_item;
	BMenuItem          *_beos_user_item;
	BMenuItem          *_riv_item;
	BMenuItem          *_jabber_org_item;
	BMenuItem          *_jabber_central_org_item;
	BMenuItem          *_jabber_view_com_item;
	BMenuItem          *_user_guide_item;
	BMenuItem          *_faq_item;
	
	BScrollView        *_chat_scroller;
	BScrollView        *_message_scroller;
	ChatTextView       *_chat;
	BetterTextView     *_message;
	BButton            *_send_message;

	BListView          *_people;
	
	BScrollView        *_scrolled_chat_pane;
	BBox               *_chat_pane;
	BScrollView        *_scrolled_people_pane;
	
	// user history
	std::deque<std::string>  _chat_history;
	std::string              _chat_buffer;
	int                 _chat_index;
	
	BFilePanel         *_fp;
	bool                _am_logging;
	FILE               *_log;

};

#endif
