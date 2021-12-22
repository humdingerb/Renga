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

	// message control
	rgb_color text_color = ui_color(B_PANEL_TEXT_COLOR);
	BFont text_font(be_plain_font);

	_message          = new BTextView("message", &text_font, &text_color, B_WILL_DRAW);
	_message_scroller = new BScrollView("message_scroller", _message, B_WILL_DRAW, false, false);

	_message->TargetedByScrollView(_message_scroller);
	_message->SetWordWrap(true);

	// editing filter for messaging
	_message->AddFilter(new EditingFilter(_message, this));

	// handle splits
	BSplitView* _split_talk = new BSplitView(B_VERTICAL);
	_split_talk->AddChild(_chat_scroller);
	_split_talk->AddChild(_message_scroller);
	_split_talk->SetItemWeight(0, 12, false);
	_split_talk->SetItemWeight(1, 1, false);
	_split_talk->SetSpacing(0);
	_split_talk->SetCollapsible(false);

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

	if (IsGroupChat())
		JabberSpeak::Instance()->SendGroupUnvitation(_group_room, _group_username);

	TalkManager::Instance()->RemoveWindow(this);
}


void TalkView::AttachedToWindow()
{
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


		case JAB_SHOW_CHATLOG: {
			// just forward to main blabber window...
			BMessage *msgForward = new BMessage(*msg);
			BlabberMainWindow::Instance()->PostMessage(msgForward);
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

		case JAB_CHAT_SENT: {
			string message = _message->Text();

			// eliminate empty messages
			if (message.empty())
				break;

			if (_session == NULL) {
				// FIXME is it ok to do this from window thread? Or should
				// we go through main app?
				gloox::MUCRoom* room = (gloox::MUCRoom*)TalkManager::Instance()
					->IsExistingWindowToGroup(GetGroupRoom());
					room->send(_message->Text());
			} else
				_session->send(_message->Text());

			// user part
			NewMessage(message);

			// GUI
			_message->ScrollToOffset(0);
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


void TalkView::AddToTalk(string username, string message, user_type type, bool highlight) {
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
	rgb_color orange = {205, 113, 57, 255};
	rgb_color message_color  = ui_color(B_PANEL_TEXT_COLOR);
	rgb_color bg_color = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color highlight_color = mix_color(message_color, orange, 200);

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
	text_run tr_thick_highlight = {0, thick, highlight_color};
	text_run tr_thin_black  = {0, thin, message_color};

	// some run array to play with (simple)
	text_run_array tra_thick_blue  = {1, {tr_thick_blue}};
	text_run_array tra_thick_red   = {1, {tr_thick_red}};
	text_run_array tra_thin_black  = {1, {tr_thin_black}};

	// construct timestamp
	char timestamp[64];
	time_t now = time(NULL);
	struct tm *time_struct = localtime(&now);

	strftime(timestamp, 63, "[%R:%S] ", time_struct);

	string time_stamp = timestamp;

	BString messageString = BString(message.c_str());

	if (BlabberSettings::Instance()->Tag("show-timestamp"))
		_chat->Insert(_chat->TextLength(), time_stamp.c_str(), time_stamp.size(), &tra_thin_black);

	if (messageString.StartsWith("/me ")) {
		messageString.ReplaceFirst("/me", username.c_str());
		if (type == MAIN_RECIPIENT)
			_chat->Insert(_chat->TextLength(), messageString, messageString.Length(), &tra_thick_blue);
		else
			_chat->Insert(_chat->TextLength(), messageString, messageString.Length(), &tra_thick_red);

		_chat->Insert(_chat->TextLength(), "\n", 1, &tra_thin_black);
		if (type == LOCAL)
			_chat->ScrollTo(0.0, _chat->Bounds().bottom);
		return;
	}

	text_run_array *this_array;
	if (type == MAIN_RECIPIENT) {
		if (!IsGroupChat() || !BlabberSettings::Instance()->Tag("exclude-groupchat-sounds"))
			SoundSystem::Instance()->PlayMessageSound();

		_chat->Insert(_chat->TextLength(), username.c_str(), username.size(), &tra_thick_blue);
		_chat->Insert(_chat->TextLength(), ": ", 2, &tra_thin_black);

		// Highlight messages when they mention the nickname
		if (highlight) {
			GenerateHyperlinkText(message, tr_thick_highlight, &this_array);
		} else {
			GenerateHyperlinkText(message, tr_thin_black, &this_array);
		}
	} else if (type == LOCAL) {
		_chat->Insert(_chat->TextLength(), username.c_str(), username.size(), &tra_thick_red);
		_chat->Insert(_chat->TextLength(), ": ", 2, &tra_thin_black);

		GenerateHyperlinkText(message, tr_thin_black, &this_array);
	} else { // SYSTEM messages
		GenerateHyperlinkText(message, tr_thick_black, &this_array);
	}
	_chat->Insert(_chat->TextLength(), message.c_str(), message.size(), this_array);
	free(this_array);

	_chat->Insert(_chat->TextLength(), "\n", 1, &tra_thin_black);
	if (type == LOCAL)
		_chat->ScrollTo(0.0, _chat->Bounds().bottom);
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


void TalkView::NewMessage(string username, string new_message, bool highlight) {
	if (username == _group_username)
		AddToTalk(username.c_str(), new_message, LOCAL, highlight);
	else
		AddToTalk(username.c_str(), new_message, MAIN_RECIPIENT, highlight);
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
