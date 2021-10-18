//////////////////////////////////////////////////
// Blabber [TalkView.cpp]
//////////////////////////////////////////////////

#include <cstdio>
#include <ctime>
#include <malloc.h>
#include <stdlib.h>

#include <Box.h>
#include <be_apps/NetPositive/NetPositive.h>
#include <FindDirectory.h>
#include <GridView.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <Roster.h>
#include <storage/Path.h>
#include <SplitView.h>

#include "support/AppLocation.h"

#include "jabber/BlabberSettings.h"
#include "jabber/CommandMessage.h"
#include "jabber/GenericFunctions.h"
#include "jabber/JabberSpeak.h"
#include "jabber/MessageRepeater.h"
#include "jabber/Messages.h"
#include "jabber/PreferencesWindow.h"
#include "jabber/SoundSystem.h"
#include "jabber/TalkListItem.h"
#include "jabber/TalkManager.h"

#include "ui/PeopleListItem.h"
#include "ui/RotateChatFilter.h"

#include "TalkView.h"

#include "gloox/mucroom.h"
#include "gloox/rostermanager.h"

#define NOTIFICATION_CHAR "√"


TalkView::TalkView(const gloox::JID *user, string group_room,
		string group_username, gloox::MessageSession* session)
	: BGroupView("<talk window>", B_VERTICAL)
	, _session(session)
{
	_am_logging = false;
	_log        = NULL;
	_chat_index = -1;

	UserID* uid = NULL;
	if (user) {
		uid = JRoster::Instance()->FindUser(*user);
	}
	_group_room     = group_room;
	_group_username = group_username;

	if (!IsGroupChat() && uid) {
		_current_status = uid->OnlineStatus();
	}

    bool bAutoOpenChatLog = BlabberSettings::Instance()->Tag("autoopen-chatlog");
	string chatlog_path = "";
	if (BlabberSettings::Instance()->Data("chatlog-path") != NULL) {
		chatlog_path = BlabberSettings::Instance()->Data("chatlog-path");
	}
	if(bAutoOpenChatLog) {
		if(0 == chatlog_path.size()) {
			BPath path;
			find_directory(B_USER_DIRECTORY, &path);
			chatlog_path = path.Path();
		}
		// assure that directory exists...
		create_directory(chatlog_path.c_str(), 0777);
		if(user != 0) {
		  chatlog_path += "/" + user->username();
		} else {
		  chatlog_path += "/" + group_room;
		}
		// start file
		_log = fopen(chatlog_path.c_str(), "a");
		_am_logging = (0 != _log);
	}

	// FILE MENU
	// status bar
	_status_view = new StatusView();
	_status_view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	_chat          = new ChatTextView("chat", B_WILL_DRAW | B_FRAME_EVENTS);
	_chat_scroller = new BScrollView("chat_scroller", _chat, B_WILL_DRAW, false, true);
	_chat->TargetedByScrollView(_chat_scroller);
	_chat->SetFontSize(12.0);
	_chat->SetWordWrap(true);
	_chat->SetStylable(true);
	_chat->MakeEditable(false);

	BGridView* _sending = new BGridView("communicate");

	// message control
	rgb_color text_color = ui_color(B_PANEL_TEXT_COLOR);
	BFont text_font(be_plain_font);

	_message          = new BTextView("message", &text_font, &text_color, B_WILL_DRAW);
	_message_scroller = new BScrollView("message_scroller", _message, B_WILL_DRAW, false, false);
	_message->TargetedByScrollView(_message_scroller);
	_message->SetWordWrap(true);

	// editing filter for messaging
	_message->AddFilter(new EditingFilter(_message, this));


	// send button
	_send_message = new BButton("send", "\xe2\x96\xb6", new BMessage(JAB_CHAT_SENT));
	_send_message->MakeDefault(true);
	_send_message->SetFlat(true);
	_send_message->SetExplicitSize(BSize(_send_message->StringWidth("WWWW"), B_SIZE_UNSET));

	// add alt-enter note

	BLayoutBuilder::Grid<>(_sending)
		.Add(_message_scroller, 0, 0)
		.Add(_send_message,     1, 0)
	.End();

	// handle splits
	BGroupView* _split_talk = new BGroupView(B_VERTICAL);
	_split_talk->AddChild(_chat_scroller);
	_split_talk->AddChild(_sending);

	_people = new BListView(NULL, B_SINGLE_SELECTION_LIST);
	_people->SetExplicitMinSize(BSize(StringWidth("Firstname M. Lastname"), B_SIZE_UNSET));
	_scrolled_people_pane = new BScrollView(NULL, _people, 0, false, true, B_PLAIN_BORDER);

	BSplitView* _split_group_people = new BSplitView(B_HORIZONTAL);
	_split_group_people->AddChild(_split_talk);
	_split_group_people->AddChild(_scrolled_people_pane);
	_split_group_people->SetItemWeight(0, 5, false);
	_split_group_people->SetItemWeight(1, 1, false);
	_split_group_people->SetSpacing(0);

	if (!IsGroupChat()) {
		_split_group_people->SetItemCollapsed(1, true);
		_split_group_people->SetSplitterSize(0);
	}

	// add GUI components to BView
	BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	layout->SetSpacing(0);

	AddChild(_split_group_people);
	AddChild(_status_view);

	_message->MakeFocus(true);

	// generate window title
	char buffer[1024];
	string user_representation;

	if (!IsGroupChat()) {
		// identify the user
		sprintf(buffer, "your identity is %s", _group_username.c_str());
		_status_view->SetMessage(buffer);
	} else if (!uid || uid->UserType() == UserID::JABBER) {
		// identify the user
		sprintf(buffer, "your identity is %s", JabberSpeak::Instance()->CurrentLogin().c_str());
		_status_view->SetMessage(buffer);
	} else {
		user_representation = uid->FriendlyName();

		if (user_representation.empty())
			user_representation = uid->JabberUsername();

		if (uid->UserType() == UserID::ICQ) {
			user_representation += " (ICQ)";

			// identify the user
			if (!AgentList::Instance()->GetAgentByService("icq")->Username().empty()) {
				sprintf(buffer, "your identity is %s", AgentList::Instance()->GetAgentByService("icq")->Username().c_str());
				_status_view->SetMessage(buffer);
			} else {
				sprintf(buffer, "you are communicating via the ICQ service");
				_status_view->SetMessage(buffer);
			}
		}
	}

	if (!IsGroupChat() && user_representation.empty()) {
		if (uid)
			user_representation = uid->FriendlyName();
		else
			user_representation = _session->target().bare();
	}

	// put Session started message
	// construct timestamp
	string message;
	message.resize(128);
	time_t now = time(NULL);
	struct tm *time_struct = localtime(&now);
	strftime(&message[0], message.size()-1, "Session started  %e %b %y [%R:%S]", time_struct);
	AddToTalk("", message.c_str(), OTHER);

	TalkManager::Instance()->StartWatchingAll(this);
}


TalkView::~TalkView() {
	string message;
	message.resize(128);
	time_t now = time(NULL);
	struct tm *time_struct = localtime(&now);
	strftime(&message[0], message.size()-1, "Session finished %e %b %y [%R:%S]\n---", time_struct);
	AddToTalk("", message.c_str(), OTHER);

	// close log cleanly if it's open
	if (_log)
		fclose(_log);

	if (IsGroupChat())
		JabberSpeak::Instance()->SendGroupUnvitation(_group_room, _group_username);

	TalkManager::Instance()->RemoveWindow(this);
}


void TalkView::AttachedToWindow()
{
	_send_message->SetTarget(this);
}


void TalkView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);

	BRect chat_rect    = _chat->Frame();
	BRect message_rect = _message->Frame();

	chat_rect.OffsetTo(B_ORIGIN);
	message_rect.OffsetTo(B_ORIGIN);

	chat_rect.InsetBy(2.0, 2.0);
	message_rect.InsetBy(2.0, 2.0);

	_chat->SetTextRect(chat_rect);
	_message->SetTextRect(message_rect);

	_chat->Invalidate();
	_chat_scroller->Invalidate();
}


void TalkView::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case JAB_CLOSE_TALKS:
		{
			RemoveSelf();
			delete this;
			break;
		}

		case B_OBSERVER_NOTICE_CHANGE:
		{
			// only for groupchat
			if (!IsGroupChat())
				break;

			switch(msg->FindInt32("be:observe_orig_what"))
			{
				case JAB_GROUP_CHATTER_ONLINE:
				{
					if (GetGroupRoom() == msg->FindString("room")) {
						AddGroupChatter(msg->FindString("username"),
							(gloox::MUCRoomAffiliation)msg->FindInt32("affiliation"));
					}
					break;
				}

				case JAB_GROUP_CHATTER_OFFLINE: {
					RemoveGroupChatter(msg->FindString("username"));
					break;
				}
				break;
			}
			break;
		}

		case JAB_START_RECORD: {
			if (_am_logging)
				break;

			// just open file panel for now
			_fp = new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL, 0, false, NULL);
			_fp->Show();

			break;
		}

		case JAB_SHOW_CHATLOG: {
			// just forward to main blabber window...
			BMessage *msgForward = new BMessage(*msg);
			BlabberMainWindow::Instance()->PostMessage(msgForward);
			break;
		}

		case B_SAVE_REQUESTED: {
			// collect data
			char abs_path[B_PATH_NAME_LENGTH];

			entry_ref dir;
			string filename;

			msg->FindRef("directory", &dir);
			filename = msg->FindString("name");

			// construct path (sigh)
			BDirectory dir_object(&dir);

			strcpy(abs_path, BPath(&dir_object, filename.c_str()).Path());

			_am_logging = true;

			// start file
			_log = fopen(abs_path, "w");

			// log existing data
			Log(_chat->Text());

			break;
		}

		case JAB_STOP_RECORD: {
			if (!_am_logging)
				break;

			// dirty work
			_am_logging = false;
			fclose(_log);

			break;
		}

		case BLAB_UPDATE_ROSTER: {
			// doesn't apply to groupchat
			if (!IsGroupChat())
				break;

			// get new status
			JRoster::Instance()->Lock();
			UserID* user = JRoster::Instance()->FindUser(_session->target());
			if (!user) {
				JRoster::Instance()->Unlock();
				break;
			}
			UserID::online_status new_status = user->OnlineStatus();

			// if we have one, check their presence
			if (_current_status != new_status) {
				char buffer[2048];

				if (_current_status == UserID::ONLINE && new_status == UserID::OFFLINE) {
					sprintf(buffer, "This user is now offline.");
					AddToTalk("", buffer, OTHER);
				} else if (_current_status == UserID::OFFLINE && new_status == UserID::ONLINE) {
					sprintf(buffer, "This user is now online.");
					AddToTalk("", buffer, OTHER);
				}
			}

			_current_status = new_status;

			JRoster::Instance()->Unlock();

			break;
		}

		case JAB_MESSAGE_1: {
			string message = BlabberSettings::Instance()->Data("message-1");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-1")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_2: {
			string message = BlabberSettings::Instance()->Data("message-2");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-2")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_3: {
			string message = BlabberSettings::Instance()->Data("message-3");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-3")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_4: {
			string message = BlabberSettings::Instance()->Data("message-4");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-4")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_5: {
			string message = BlabberSettings::Instance()->Data("message-5");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-5")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_6: {
			string message = BlabberSettings::Instance()->Data("message-6");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-6")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_7: {
			string message = BlabberSettings::Instance()->Data("message-7");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-7")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_8: {
			string message = BlabberSettings::Instance()->Data("message-8");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-8")) {
				// network part
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_MESSAGE_9: {
			string message = BlabberSettings::Instance()->Data("message-9");

			if (!message.empty() && BlabberSettings::Instance()->Tag("fire-9")) {
				_session->send(message);

				// client part
				NewMessage(message);
			} else {
				_message->Insert(message.c_str());
			}

			break;
		}

		case JAB_CHAT_SENT: {
			string message = _message->Text();

			// eliminate empty messages
			if (message.empty())
				break;

			if (!CommandMessage::IsCommand(message)
				|| CommandMessage::IsLegalCommand(message)) {
				if (_session == NULL) {
					// FIXME is it ok to do this from window thread? Or should
					// we go through main app?
					gloox::MUCRoom* room = (gloox::MUCRoom*)TalkManager::Instance()
						->IsExistingWindowToGroup(GetGroupRoom());
					room->send(_message->Text());
				} else
					_session->send(_message->Text());
			}

			// user part
			NewMessage(message);

			// GUI
			_message->SetText("");
			_message->MakeFocus(true);

			break;
		}

		case JAB_CLOSE_CHAT: {
			RemoveSelf();
			delete this;
			break;
		}

        case JAB_FOCUS_BUDDY: {
	         BlabberMainWindow::Instance()->Activate();
             break;
        }

		case kIncomingMessage:
		{
			AddToTalk(OurRepresentation().c_str(), msg->FindString("content"),
				LOCAL);
		}
	}
}


string TalkView::OurRepresentation() {
	// use friendly name if you have it
	string user = JabberSpeak::Instance()->CurrentRealName();

	// and if not :)
	if (user.empty())
		user = JabberSpeak::Instance()->CurrentLogin();

	return user;
}


void TalkView::AddToTalk(string username, string message, user_type type) {
	// transform local identity
	if (IsGroupChat() && type == LOCAL)
		username = _group_username;

	// history
	if (type == LOCAL) {
		// reset chat history index
		_chat_index = -1;

		// add latest
		_chat_history.push_front(message);

		// prune end
		if (_chat_history.size() > 50)
			_chat_history.pop_back();
	}

	// ignore empty messages
	if (message.empty())
		return;

	// prune trailing whitespace
	while (!message.empty() && isspace(message[message.size() - 1]))
		message.erase(message.size() - 1);

	// create the thin (plain) and thick (bold) font
	BFont thin(be_plain_font);
	BFont thick(be_bold_font);

	// some colors to play with
	rgb_color blue   = {0, 0, 255, 255};
	rgb_color red    = {255, 0, 0, 255};
	rgb_color message_color  = ui_color(B_PANEL_TEXT_COLOR);
	rgb_color bg_color = ui_color(B_PANEL_BACKGROUND_COLOR);

	// TODO figure out a goood threshold here, this seems to work for me,
	// but it may not for others
	if (abs(blue.Brightness() - bg_color.Brightness()) < 45)
		blue = { 128, 137, 252, 255 };

	if (abs(red.Brightness() - bg_color.Brightness()) < 45)
		red = { 249, 84, 87, 255 };

	// some runs to play with
	text_run tr_thick_blue  = {0, thick, blue};
	text_run tr_thick_red   = {0, thick, red};
	text_run tr_thick_black = {0, thick, message_color};
	text_run tr_thin_black  = {0, thin, message_color};

	// some run array to play with (simple)
	text_run_array tra_thick_blue  = {1, {tr_thick_blue}};
	text_run_array tra_thick_red   = {1, {tr_thick_red}};
	text_run_array tra_thick_black = {1, {tr_thick_black}};
	text_run_array tra_thin_black  = {1, {tr_thin_black}};

	// add to end of conversation
	bool                         is_command = CommandMessage::IsCommand(message);
 	CommandMessage::command_type comm_type  = CommandMessage::ConvertCommandToMessage(message, username);

	// construct timestamp
	char timestamp[64];
	time_t now = time(NULL);
	struct tm *time_struct = localtime(&now);

	strftime(timestamp, 63, "[%R:%S] ", time_struct);

	string time_stamp = timestamp;

	if (comm_type == CommandMessage::NORMAL) {
		if (type == MAIN_RECIPIENT) {
			if (_chat->TextLength() > 0) {
				if (!IsGroupChat() || !BlabberSettings::Instance()->Tag("exclude-groupchat-sounds")) {
					SoundSystem::Instance()->PlayMessageSound();
				}
			}

			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), &tra_thick_blue);
		} else if (type == LOCAL || (IsGroupChat() && GetGroupUsername() == username)) {
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), &tra_thick_red);
		} else {
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), &tra_thick_black);
		}

		// log?
		Log(message.c_str());
	} else if (comm_type == CommandMessage::BAD_SYNTAX) {
		if (type == LOCAL || (IsGroupChat() && GetGroupUsername() == username)) {
			// this command was illegal and now it belongs to the system
			type = OTHER;

			// print usage (most likely)
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), &tra_thick_black);

			// log?
			Log(message.c_str());
		}
	} else if (is_command && comm_type == CommandMessage::NOT_A_COMMAND) {
		if (type == LOCAL || (IsGroupChat() && GetGroupUsername() == username)) {
			message = "You specified an illegal command.\n";

			// this command was illegal and now it belongs to the system
			type = OTHER;

			// print usage (most likely)
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), &tra_thick_black);

			// log?
			Log(message.c_str());
		}
	} else {
		if (type == MAIN_RECIPIENT) {
			if (comm_type != CommandMessage::NORMAL_ALERT && SoundSystem::Instance()->AlertSound() != "<none>" && _chat->TextLength() > 0) {
				if (!IsGroupChat() || !BlabberSettings::Instance()->Tag("exclude-groupchat-sounds")) {
					SoundSystem::Instance()->PlayMessageSound();
				}
			}

			if (BlabberSettings::Instance()->Tag("show-timestamp")) {
				_chat->Insert(_chat->TextLength(), time_stamp.c_str(), time_stamp.size(), &tra_thin_black);

				// log?
				Log(time_stamp.c_str());
			}

			_chat->Insert(_chat->TextLength(), username.c_str(), username.size(), &tra_thick_blue);

			// log?
			Log(username.c_str());

			_chat->Insert(_chat->TextLength(), ": ", 2, &tra_thin_black);

			// log?
			Log(": ");

			text_run_array *this_array;
			GenerateHyperlinkText(message, tr_thin_black, &this_array);
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), this_array);

			free(this_array);

			// log?
			Log(message.c_str());

			_chat->Insert(_chat->TextLength(), "\n", 1, &tra_thin_black);

			// log?
			Log("\n");
		} else if (type == LOCAL || (IsGroupChat() && GetGroupUsername() == username)) {
			if (BlabberSettings::Instance()->Tag("show-timestamp")) {
				_chat->Insert(_chat->TextLength(), time_stamp.c_str(), time_stamp.size(), &tra_thin_black);

				// log?
				Log(time_stamp.c_str());
			}

			_chat->Insert(_chat->TextLength(), username.c_str(), username.size(), &tra_thick_red);

			// log?
			Log(username.c_str());

			_chat->Insert(_chat->TextLength(), ": ", 2, &tra_thin_black);

			// log?
			Log(": ");

			text_run_array *this_array;
			GenerateHyperlinkText(message, tr_thin_black, &this_array);
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), this_array);

			free(this_array);

			// log?
			Log(message.c_str());

			_chat->Insert(_chat->TextLength(), "\n", 1, &tra_thin_black);

			// log?
			Log("\n");
		} else {
			// system message
			_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), &tra_thick_black);

			// log?
			Log(message.c_str());

			_chat->Insert(_chat->TextLength(), "\n", 1, &tra_thick_black);

			// log?
			Log("\n");
		}

		if (comm_type == CommandMessage::NORMAL_ALERT) {
			if (!BlabberSettings::Instance()->Tag("suppress-alert")) {
				// play sound
				if (type != LOCAL) {
					SoundSystem::Instance()->PlayAlertSound();
				}
			}
		}
	}

	_chat->ScrollTo(0.0, _chat->Bounds().bottom);
}


void TalkView::Log(const char *buffer) {
	if (_am_logging) {
		fwrite(buffer, sizeof(char), strlen(buffer), _log);
		fflush(_log);
	}
}


void TalkView::NewMessage(string new_message) {
	if (IsGroupChat())
		return;

	gloox::RosterManager* rm = JabberSpeak::Instance()->GlooxClient()->rosterManager();
	gloox::RosterItem* item = rm->getRosterItem(_session->target());
	if (item && !item->name().empty()) {
		AddToTalk(item->name().c_str(), new_message, LOCAL);
	} else {
		AddToTalk(_session->target().bare().c_str(), new_message, LOCAL);
	}
}


void TalkView::NewMessage(string username, string new_message) {
	if (username == _group_username)
		AddToTalk(username.c_str(), new_message, LOCAL);
	else
		AddToTalk(username.c_str(), new_message, MAIN_RECIPIENT);
}


const gloox::JID& TalkView::GetUserID() {
	if (_session == NULL)
		debugger("Getting user ID not possible for group chat");

	return _session->target();
}


string TalkView::GetGroupRoom() {
	return _group_room;
}


string TalkView::GetGroupUsername() {
	return _group_username;
}


bool TalkView::NewlinesAllowed() {
	return false;
}


int TalkView::CountHyperlinks(string message) {
	string::size_type curr_pos = 0, link_start, link_end;
	string::size_type find1, find2, find3;

	// keep count
	int link_count = 0;

	// find next link
	link_start = message.find("http://", curr_pos);

	find1 = message.find("ftp://", curr_pos);
	if (find1 != string::npos && (link_start == string::npos || find1 < link_start)) {
		link_start = find1;
	}

	find2 = message.find("www.", curr_pos);
	if (find2 != string::npos && (link_start == string::npos || find2 < link_start)) {
		// ignore if it's not at the beginning or has no whitespace
		if (find2 > 0 && isalnum(message[find2 - 1])) {
			// do nothing
 		} else if (isspace(message[find2 + 4]) || message[find2 + 4] == '.') {
			// do nothing
		} else {
			link_start = find2;
		}
	}

	find3 = message.find("ftp.", curr_pos);
	if (find3 != string::npos && (link_start == string::npos || find3 < link_start)) {
		// ignore if it's not at the beginning or has no whitespace
		if (find3 > 0 && isalnum(message[find3 - 1])) {
			// do nothing
 		} else if (isspace(message[find3 + 4]) || message[find3 + 4] == '.') {
			// do nothing
		} else {
			link_start = find3;
		}
	}

	while (link_start != string::npos) {
		// find whitespace or end
		link_end = message.find_first_of(" \t\r\n", link_start);

		if (link_end == string::npos)
			link_end = message.size() - 1;

		// prune punctuation
		while (link_start < link_end) {
			if (message[link_end] == ',' || message[link_end] == '!' || message[link_end] == '.' || message[link_end] == ')' || message[link_end] == ';' || message[link_end] == ']' || message[link_end] == '>' || message[link_end] == '\'' || message[link_end] == '"') {
				--link_end;
			} else {
				break;
			}
		}

		if (link_start < link_end) {
			++link_count;
		}

		curr_pos = link_end + 1;

		// find next link
		link_start = message.find("http://", curr_pos);

		find1 = message.find("ftp://", curr_pos);
		if (find1 != string::npos && (link_start == string::npos || find1 < link_start))
			link_start = find1;

		find2 = message.find("www.", curr_pos);
		if (find2 != string::npos && (link_start == string::npos || find2 < link_start)) {
			// ignore if it's not at the beginning or has no whitespace
			if (find2 > 0 && isalnum(message[find2 - 1])) {
				// do nothing
	 		} else if (isspace(message[find2 + 4]) || message[find2 + 4] == '.') {
				// do nothing
			} else {
				link_start = find2;
			}
		}

		find3 = message.find("ftp.", curr_pos);
		if (find3 != string::npos && (link_start == string::npos || find3 < link_start)) {
			// ignore if it's not at the beginning or has no whitespace
			if (find3 > 0 && isalnum(message[find3 - 1])) {
				// do nothing
	 		} else if (isspace(message[find3 + 4]) || message[find3 + 4] == '.') {
				// do nothing
			} else {
				link_start = find3;
			}
		}
	}

	return link_count;
}


void TalkView::GenerateHyperlinkText(string message, text_run standard, text_run_array **tra) {
	int link_count = CountHyperlinks(message);
	string::size_type find1, find2, find3;
	int link_index = 0;

	// no links?
	if (link_count == 0) {
		*tra = (text_run_array *)malloc(sizeof(text_run_array));

		(*tra)->count = 1;
		(*tra)->runs[0].offset = standard.offset;
		(*tra)->runs[0].font = standard.font;
		(*tra)->runs[0].color = standard.color;

		return;
	}

	*tra = (text_run_array *)malloc(sizeof(text_run_array) + (sizeof(text_run) * (link_count * 2 - 1)));
	(*tra)->count = link_count * 2;

	string::size_type curr_pos = 0, link_start, link_end;

	// find next link
	link_start = message.find("http://", curr_pos);

	find1 = message.find("ftp://", curr_pos);
	if (find1 != string::npos && (link_start == string::npos || find1 < link_start))
		link_start = find1;

	find2 = message.find("www.", curr_pos);
	if (find2 != string::npos && (link_start == string::npos || find2 < link_start)) {
		// ignore if it's not at the beginning or has no whitespace
		if (find2 > 0 && isalnum(message[find2 - 1])) {
			// do nothing
 		} else if (isspace(message[find2 + 4]) || message[find2 + 4] == '.') {
			// do nothing
		} else {
			link_start = find2;
		}
	}

	find3 = message.find("ftp.", curr_pos);
	if (find3 != string::npos && (link_start == string::npos || find3 < link_start)) {
		// ignore if it's not at the beginning or has no whitespace
		if (find3 > 0 && isalnum(message[find3 - 1])) {
			// do nothing
 		} else if (isspace(message[find3 + 4]) || message[find3 + 4] == '.') {
			// do nothing
		} else {
			link_start = find3;
		}
	}

	while (link_start != string::npos) {
		// find whitespace or end
		link_end = message.find_first_of(" \t\r\n", link_start);

		if (link_end == string::npos) {
			link_end = message.size() - 1;
		}

		// prune punctuation
		while (link_start < link_end) {
			if (message[link_end] == ',' || message[link_end] == '!' || message[link_end] == '.' || message[link_end] == ')' || message[link_end] == ';' || message[link_end] == ']' || message[link_end] == '>' || message[link_end] == '?' || message[link_end] == '\'' || message[link_end] == '"') {
				--link_end;
			} else {
				break;
			}
		}

		// add hyperlink
		if (link_start < link_end) {
			BFont thin(be_plain_font);
			rgb_color purple = {192, 0, 192, 255};

			(*tra)->runs[link_index].offset = link_start;
			(*tra)->runs[link_index].font = thin;
			(*tra)->runs[link_index].color = purple;

			(*tra)->runs[link_index + 1].offset = link_end + 1;
			(*tra)->runs[link_index + 1].font = standard.font;
			(*tra)->runs[link_index + 1].color = standard.color;
		}

		curr_pos = link_end + 1;

		if (curr_pos >= message.size()) {
			break;
		}

		// find next link
		link_start = message.find("http://", curr_pos);

		find1 = message.find("ftp://", curr_pos);
		if (find1 != string::npos && (link_start == string::npos || find1 < link_start)) {
			link_start = find1;
		}

		find2 = message.find("www.", curr_pos);
		if (find2 != string::npos && (link_start == string::npos || find2 < link_start)) {
			// ignore if it's not at the beginning or has no whitespace
			if (find2 > 0 && isalnum(message[find2 - 1])) {
				// do nothing
	 		} else if (isspace(message[find2 + 4]) || message[find2 + 4] == '.') {
				// do nothing
			} else {
				link_start = find2;
			}
		}

		find3 = message.find("ftp.", curr_pos);
		if (find3 != string::npos && (link_start == string::npos || find3 < link_start)) {
			// ignore if it's not at the beginning or has no whitespace
			if (find3 > 0 && isalnum(message[find3 - 1])) {
				// do nothing
	 		} else if (isspace(message[find3 + 4]) || message[find3 + 4] == '.') {
				// do nothing
			} else {
				link_start = find3;
			}
		}

		link_index += 2;
	}
}


static int compareStrings(const char* a, const char* b)
{
	// FIXME use ICU locale aware comparison instead
	int icompare = strcasecmp(a, b);
	if (icompare != 0)
		return icompare;

	// In case the names are case-insensitive-equal, still sort them in a
	// predictible way
	return strcmp(a, b);
}


void TalkView::AddGroupChatter(string user, gloox::MUCRoomAffiliation affiliation) {
	int i;

	// create a new entry
	PeopleListItem *people_item = new PeopleListItem(user, affiliation);

	// exception
	if (_people->CountItems() == 0) {
		// add the new user
		_people->AddItem(people_item);
		return;
	}

	// add it to the list
	// FIXME we should binary search for the correct position
	for (i=0; i < _people->CountItems(); ++i) {
		PeopleListItem *iterating_item = dynamic_cast<PeopleListItem *>(_people->ItemAt(i));

		int compare = compareStrings(iterating_item->User().c_str(), user.c_str());

		if (compare == 0) {
			// Update existing user
			// FIXME affiliation might have changed, refresh it
			_people->InvalidateItem(i);
		} else if (compare > 0) {
			// add the new user in the middle
			_people->AddItem(people_item, i);
		} else if (i == (_people->CountItems() - 1)) {
			// add the new user at the end
			_people->AddItem(people_item);
		} else {
			// continue searching for the correct place
			continue;
		}

		break;
	}
}


void TalkView::RemoveGroupChatter(string username) {
	// remove user
	for (int i=0; i < _people->CountItems(); ++i) {
		if (dynamic_cast<PeopleListItem *>(_people->ItemAt(i))->User() == username)
			_people->RemoveItem(i);
	}
}


void TalkView::RevealPreviousHistory() {
	// boundary
	if (_chat_index == 49 || _chat_index == ((int)_chat_history.size() - 1))
		return;

	if (_chat_index == -1)
		_chat_buffer = _message->Text();

	// go back
	++_chat_index;

	// update text
	_message->SetText(_chat_history[_chat_index].c_str());
}


void TalkView::RevealNextHistory() {
	// boundary
	if (_chat_index == -1)
		return;

	// go forward
	--_chat_index;

	// last buffer
	if (_chat_index == -1) {
		_message->SetText(_chat_buffer.c_str());
	} else {
		// update text
		_message->SetText(_chat_history[_chat_index].c_str());
	}
}


bool
TalkView::IsGroupChat()
{
	return !_group_room.empty();
}


void
TalkView::SetStatus(std::string message)
{
	_status_view->SetMessage(message);
}
