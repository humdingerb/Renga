//////////////////////////////////////////////////
// Blabber [TalkWindow.h]
//     A session with another user.
//////////////////////////////////////////////////

#pragma once

#include <gloox/message.h>
#include <gloox/messagehandler.h>

#include <cstdio>
#include <deque>
#include <string>

#include <interface/Box.h>
#include <interface/Button.h>
#include <interface/CheckBox.h>
#include <GroupView.h>
#include <interface/ListView.h>
#include <interface/MenuBar.h>
#include <interface/MenuItem.h>
#include <interface/ScrollView.h>
#include <interface/StringView.h>
#include <String.h>
#include <interface/TextView.h>
#include <interface/Window.h>
#include <storage/FilePanel.h>

#include "jabber/BetterTextView.h"
#include "jabber/ChatTextView.h"
#include "jabber/ChatWidget.h"
#include "jabber/StatusView.h"
#include "jabber/UserID.h"

#include "ui/EditingFilter.h"

class TalkView : public BGroupView {
public:
	enum                 user_type {MAIN_RECIPIENT, LOCAL, OTHER};

public:
						TalkView(const gloox::JID *user, std::string group_room,
							std::string group_username,
							gloox::MessageSession* session);
						~TalkView();

	void				AttachedToWindow() override;
	void                 FrameResized(float width, float height) override;
	void                 MessageReceived(BMessage *msg) override;

	void                 Log(const char *buffer);
	std::string          OurRepresentation();
	void                 AddToTalk(std::string username, std::string message, user_type type);
	void                 NewMessage(std::string new_message);
	void                 NewMessage(std::string username, std::string new_message);

	bool                 NewlinesAllowed();

	const gloox::JID&    GetUserID();
	std::string          GetGroupRoom();
	std::string          GetGroupUsername();

	int                  CountHyperlinks(std::string message);
	void                 GenerateHyperlinkText(std::string message, text_run standard, text_run_array **tra);

	void                 AddGroupChatter(std::string username, gloox::MUCRoomAffiliation);
	void                 RemoveGroupChatter(std::string username);

	void                 RevealPreviousHistory();
	void                 RevealNextHistory();

	bool				IsLogging() { return _am_logging; }
	bool				IsGroupChat();

	void				SetStatus(std::string message);

	// gloox MessageHandler
	void handleMessage(const gloox::Message&, gloox::MessageSession*);

private:
    BString				originalWindowTitle;

	std::string            _group_room;
	std::string            _group_username;
	UserID::online_status  _current_status;
	gloox::MessageSession* _session;

	// GUI
	StatusView         *_status_view;

	BScrollView        *_chat_scroller;
	BScrollView        *_message_scroller;
	ChatTextView       *_chat;
	BetterTextView     *_message;
	BButton            *_send_message;

	BListView          *_people;

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
