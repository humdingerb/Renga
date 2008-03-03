//////////////////////////////////////////////////
// Blabber [TalkWindow.h]
//     A session with another user.
//////////////////////////////////////////////////

#ifndef TALK_WINDOW_H
#define TALK_WINDOW_H

#ifndef __CSTDIO__
	#include <cstdio>
#endif

#ifndef __DEQUE__
	#include <deque>
#endif

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

#ifndef _LIST_VIEW_H
	#include <interface/ListView.h>
#endif

#ifndef _MENU_BAR_H
	#include <interface/MenuBar.h>
#endif

#ifndef _MENU_ITEM_H
	#include <interface/MenuItem.h>
#endif

#ifndef _SCROLL_VIEW_H
	#include <interface/ScrollView.h>
#endif

#ifndef _STRING_VIEW_H
	#include <interface/StringView.h>
#endif

#ifndef _TEXT_VIEW_H
	#include <interface/TextView.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

#ifndef _FILE_PANEL_H
	#include <storage/FilePanel.h>
#endif

#ifndef BETTER_TEXT_VIEW_H
	#include "BetterTextView.h"
#endif

#ifndef CHAT_TEXT_VIEW_H
	#include "ChatTextView.h"
#endif

#ifndef CHAT_WIDGET_H
	#include "ChatWidget.h"
#endif

#ifndef EDITING_FILTER_H
	#include "EditingFilter.h"
#endif

#ifndef _SPLIT_PANE_VIEW_H
	#include "SplitPane.h"
#endif

#ifndef _STATUS_VIEW_H
	#include "StatusView.h"
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

class TalkWindow : public BWindow {
public:
	enum                 talk_type {MESSAGE, CHAT, GROUP};
	enum                 user_type {MAIN_RECIPIENT, LOCAL, OTHER};

public:
	static float         x_placement_offset;
	static float         y_placement_offset;
	
public:  
	                     TalkWindow(talk_type type, const UserID *user, string group_room, string group_username, bool follow_focus_rules = false);
	                    ~TalkWindow();

	talk_type            Type();

	void                 FrameResized(float width, float height);
	void                 MenusBeginning();
	void                 MessageReceived(BMessage *msg);
	bool                 QuitRequested();
	
	void                 Log(const char *buffer);
	string               OurRepresentation();
	void                 AddToTalk(string username, string message, user_type type);
	void                 NewMessage(string new_message);
	void                 NewMessage(string username, string new_message);
	void                 SetThreadID(string id);

	bool                 NewlinesAllowed();

	const UserID        *GetUserID();
	string               GetGroupRoom();
	string               GetGroupUsername();

	int                  CountHyperlinks(string message);
	void                 GenerateHyperlinkText(string message, text_run standard, text_run_array **tra);

	void                 AddGroupChatter(string username);
	void                 RemoveGroupChatter(string username);

	void                 RevealPreviousHistory();		
	void                 RevealNextHistory();		

private:
	const UserID          *_user;
	string                 _group_room;
	string                 _group_username;
	UserID::online_status  _current_status;
	talk_type              _type;
	string                 _thread;
	
	// GUI
	BView              *_full_view;

	StatusView         *_status_view;
	BBox               *_sending;
	
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
	BTextView          *_enter_note;
	BCheckBox          *_command_enter;
	BButton            *_send_message;

	BListView          *_people;
	
	SplitPane          *_split_talk;
	SplitPane          *_split_group_people;

	BScrollView        *_scrolled_chat_pane;
	BBox               *_chat_pane;
	BScrollView        *_scrolled_people_pane;
	
	// user history
	deque<string>     _chat_history;
	string              _chat_buffer;
	int                 _chat_index;
	
	BFilePanel         *_fp;
	bool                _am_logging;
	FILE               *_log;
};

#endif