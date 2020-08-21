//////////////////////////////////////////////////
// Blabber [BlabberMainWindow.cpp]
//////////////////////////////////////////////////

#include "BlabberMainWindow.h"

#include <gloox/jid.h>

#include "../support/AppLocation.h"
#include "BlabberSettings.h"
#include "BuddyWindow.h"
#include "BuddyInfoWindow.h"
#include "../ui/ChangeNameWindow.h"
#include "CustomStatusWindow.h"
#include "GenericFunctions.h"
#include "JabberSpeak.h"
#include "Messages.h"
#include "MessageRepeater.h"
#include "../ui/ModalAlertFactory.h"
#include "PreferencesWindow.h"
#include "../ui/RegisterAccountWindow.h"
#include "RosterItem.h"
#include "RotateChatFilter.h"
#include "SendTalkWindow.h"
#include "TalkManager.h"
#include "../ui/TalkView.h"

#include <Application.h>
#include <CardLayout.h>
#include <Deskbar.h>
#include <FindDirectory.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <be_apps/NetPositive/NetPositive.h>
#include <Path.h>
#include <Roster.h>
#include <ScrollView.h>
#include <String.h>

#include <algorithm>
#include <cstdio>
#include <stdlib.h>


enum {
	kCreateAccount = 'crea'
};


BlabberMainWindow *BlabberMainWindow::_instance = NULL;

BlabberMainWindow *BlabberMainWindow::Instance() {
	BlabberSettings *settings = BlabberSettings::Instance();
	
	if (_instance == NULL && !settings->Data("no-window-on-startup")) {
		float main_window_width, main_window_height;

		// determine what the width and height of the window should be
		if (settings->Data("main-window-width") && settings->Data("main-window-height")) {
			main_window_width  = atof(settings->Data("main-window-width"));
			main_window_height = atof(settings->Data("main-window-height"));
		} else {
			// default
			main_window_width  = 210;
			main_window_height = 332; 
		}
		
		// create window frame position
		BRect frame(GenericFunctions::CenteredFrame(main_window_width, main_window_height));

		// poition window to last known position
		if (settings->Data("main-window-left") && settings->Data("main-window-top")) {
			frame.OffsetTo(BPoint(atof(settings->Data("main-window-left")), atof(settings->Data("main-window-top"))));
		}

		// create window singleton
		_instance = new BlabberMainWindow(frame);
	}
	
	return _instance;
}

BlabberMainWindow::~BlabberMainWindow() {
	// remove self from message family
	MessageRepeater::Instance()->RemoveTarget(this);

	// remove deskbar icon
//	BDeskbar db;
//	db.RemoveItem(_deskbar_id);	

	_instance = NULL;
}

void BlabberMainWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kCreateAccount:
		{
			(new RegisterAccountWindow(this))->Show();
			break;
		}

		// transplanted from LoginWindow
		case JAB_LOGIN: {
			// must pass validation
			if (!ValidateLogin()) {
				break;
			}

			// switch out views
			_login_login->MakeDefault(false);
			BCardLayout* cl = (BCardLayout*)GetLayout();
			cl->SetVisibleItem((int32)0);

			// connect with current username or register new account
			JabberSpeak::Instance()->SendConnect(_login_username->Text(), _login_password->Text(), _login_realname->Text());

			break;
		}
		
		case B_ABOUT_REQUESTED: {
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		}

		case JAB_QUIT: {
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}

		case JAB_PREFERENCES: {
			PreferencesWindow::Instance()->Show();
			PreferencesWindow::Instance()->Activate();
			break;
		}

		case JAB_CONNECT: {
			_status_view->SetMessage("connecting");
			JabberSpeak::Instance()->SendConnect("", "", "", true);

			break;
		}

		case JAB_RECONNECTING: {
			// we are connecting
			_status_view->SetMessage("reconnecting");

			break;
		}

		case JAB_LOGGED_IN: {
			// we just logged in
			_status_view->SetMessage("connected as " + JabberSpeak::Instance()->CurrentLogin());

			// save these settings
			BlabberSettings::Instance()->SetData("last-realname", _login_realname->Text());
			BlabberSettings::Instance()->SetData("last-login", _login_username->Text());
			BlabberSettings::Instance()->SetData("last-password", _login_password->Text());
			BlabberSettings::Instance()->SetTag("auto-login", _login_auto_login->Value());
			BlabberSettings::Instance()->WriteToFile();

			break;
		}
		
		case JAB_DISCONNECT: {
			JabberSpeak::Instance()->SendDisconnect();
			_status_view->SetMessage("disconnecting");
			JabberSpeak::Instance()->Reset();

			break;
		}

		case JAB_DISCONNECTED: {
			JRoster::Instance()->RefreshRoster();

			break;
		}

		case JAB_RIV: {
			string jabber_org = "http://github.com/haikuarchives/renga"; 
			
			char *argv[] = {const_cast<char *>(jabber_org.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", jabber_org.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}

		case JAB_BE_USERS: {
			string jabber_org = "http://home.t-online.de/home/sascha.offe/jabber.html"; 
			
			char *argv[] = {const_cast<char *>(jabber_org.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", jabber_org.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}
				
		case JAB_JABBER_ORG: {
			string jabber_org = "http://www.jabber.org"; 
			
			char *argv[] = {const_cast<char *>(jabber_org.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", jabber_org.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}

		case JAB_JABBER_CENTRAL_ORG: {
			string jabber_org = "http://www.jabbercentral.org"; 
			
			char *argv[] = {const_cast<char *>(jabber_org.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", jabber_org.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}

		case JAB_JABBER_VIEW_COM: {
			string jabber_org = "http://www.jabberview.com"; 
			
			char *argv[] = {const_cast<char *>(jabber_org.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", jabber_org.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}

		case JAB_FAQ: {
			string user_guide = "http://www.users.uswest.net/~jblanco/jabber-faq.html";
			
			char *argv[] = {const_cast<char *>(user_guide.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", user_guide.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}

		case JAB_USER_GUIDE: {
			string user_guide = AppLocation::Instance()->AbsolutePath("resources/user-guide/user_guide.html");
			
			char *argv[] = {const_cast<char *>(user_guide.c_str()), NULL};
			if (!be_roster->IsRunning("text/html"))
				be_roster->Launch("text/html", 1, argv);
			else {
				BMessenger messenger("text/html");
				BMessage msg(B_NETPOSITIVE_OPEN_URL);
				msg.AddString("be:url", user_guide.c_str());
				messenger.SendMessage(&msg);
			}
	
			break;
		}

		case BLAB_UPDATE_ROSTER: {
			// a message that the roster singleton was updated
			_roster->UpdateRoster();
	
			break;
		}

		case JAB_SUBSCRIBE_PRESENCE: {
			RosterItem *item = _roster->CurrentItemSelection();
			
			// if there was a user selected (shouldn't be otherwise)
			if (item) {
				// don't send subscription request if we have it already
				if (!item->GetUserID()->HaveSubscriptionTo()) {
					JabberSpeak::Instance()->SendSubscriptionRequest(item->GetUserID()->JabberHandle());
				}
			}
			
			break;
		}

		case JAB_UNSUBSCRIBE_PRESENCE: {
			RosterItem *item = _roster->CurrentItemSelection();
			
			// if there was a user selected (shouldn't be otherwise)
			if (item) {
				// don't send unsubscription request if we don't have it already
				if (item->GetUserID()->HaveSubscriptionTo()) {
					JabberSpeak::Instance()->SendUnsubscriptionRequest(item->GetUserID()->Handle());
				}
			}
			
			break;
		}

		case JAB_OPEN_CHAT_WITH_DOUBLE_CLICK: {
			// works in combination with JAB_OPEN_CHAT case
			if (!BlabberSettings::Instance()->Tag("enable-double-click")) {
				break;
			}
		}
		__attribute__ ((fallthrough));
		
		case JAB_OPEN_CHAT: {
			// if there's a current selection, begin chat with that user
			RosterItem *item = _roster->CurrentItemSelection();
			
			if (item != NULL) {
				const UserID *user = item->GetUserID();

				// open chat window
				TalkManager::Instance()->CreateTalkSession(gloox::Message::Chat,
					&user->JID(), "", "", NULL);
			}

			// if there's a current selection, begin chat with that group
			BookmarkItem *bookmark = _roster->CurrentBookmarkSelection();
			
			if (bookmark != NULL) {
				const gloox::JID& group = bookmark->GetUserID();
				const gloox::ConferenceListItem* info
					= BookmarkManager::Instance().GetBookmark(group.full().c_str());

				// open chat window
				TalkManager::Instance()->CreateTalkSession(gloox::Message::Groupchat,
					NULL, group.full(), info->nick, NULL);

				// Enable autojoin if needed
				BookmarkManager::Instance().SetBookmark(group.full().c_str(),
					info->nick.c_str(), bookmark->Text(), true);
			}
			
			break;
		}

		case JAB_SHOW_CHATLOG: {
			BString path;
			// try to get the chatlog path from message
			if(B_OK != msg->FindString("path", &path)) {
				// no path provided with message? try to open history for selected user...
  				RosterItem *item = _roster->CurrentItemSelection();
				if (item != NULL) {
					const UserID *user = item->GetUserID();
					string chatlog_path = BlabberSettings::Instance()->Data("chatlog-path");
					if(0 == chatlog_path.size()) {
						BPath path;
						find_directory(B_USER_DIRECTORY, &path);
						chatlog_path = path.Path();
					}
					chatlog_path += "/" + user->JabberHandle();
					path = chatlog_path.c_str();
				}
			}
			// now attemt to open history file with default application for "text/x-jabber-chatlog" MIME type
			if(path.Length() > 0) {
				BEntry entry(path.String());
				entry_ref ref;
				entry.GetRef(&ref);
				BMessage *msgRefs = new BMessage(B_REFS_RECEIVED);
				msgRefs->AddRef("refs", &ref);
				be_roster->Launch("text/plain", msgRefs);
				//be_roster->Launch(path.String());
			}
			break;
		}

		case JAB_OPEN_NEW_CHAT: {
			(new SendTalkWindow(gloox::Message::Chat))->Show();
			break;
		}

		case JAB_OPEN_NEW_GROUP_CHAT: {
			(new SendTalkWindow(gloox::Message::Groupchat))->Show();
			break;
		}

		case JAB_OPEN_MESSAGE: {
			// if there's a current selection, begin message with that user
			RosterItem *item = _roster->CurrentItemSelection();

			const UserID *user = item->GetUserID();

			// open message window
			TalkManager::Instance()->CreateTalkSession(gloox::Message::Normal, &user->JID(), "", "", NULL);
			
			break;
		}

		case JAB_OPEN_NEW_MESSAGE: {
			(new SendTalkWindow(gloox::Message::Normal))->Show();
			break;
		}

		case JAB_OPEN_ADD_BUDDY_WINDOW: {
			// open buddy window
			BuddyWindow::Instance()->Show();
			break;
		}

		case JAB_OPEN_EDIT_BUDDY_WINDOW: {
			// pick out user to be edited from RosterView
			RosterItem *item = _roster->CurrentItemSelection();
			
			if (item != NULL) {
				// open edit buddy window
				(new ChangeNameWindow(item->GetUserID()->JID(), item->GetUserID()->FriendlyName().c_str()))->Show();
			}

			BookmarkItem *bookmark = _roster->CurrentBookmarkSelection();

			if (bookmark != NULL) {
				// open edit buddy window
				(new ChangeNameWindow(bookmark->GetUserID(), bookmark->Text()))->Show();
			}

			break;
		}

		case JAB_USER_INFO: {
			// pick out user to be analyzed
			RosterItem *item = _roster->CurrentItemSelection();
			
			if (item != NULL) {
				// pick out user
				UserID *user = const_cast<UserID *>(item->GetUserID());

				// open edit buddy window
				(new BuddyInfoWindow(user))->Show();
			}

			break;
		}

		case JAB_REMOVE_BUDDY: {
			// pick out user to be removed from RosterView
			RosterItem *item = _roster->CurrentItemSelection();
			
			if (item != NULL) {
				// pick out user
				const UserID *user = item->GetUserID();

				// remove the user
				JabberSpeak::Instance()->RemoveFromRoster(user);
			}

			BookmarkItem *bookmark = _roster->CurrentBookmarkSelection();
			if (bookmark)
				BookmarkManager::Instance().RemoveBookmark(bookmark->GetUserID().full().c_str());

			break;
		}

		case BLAB_AVAILABLE_FOR_CHAT: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::Chat);
			_chat_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "chat");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_DO_NOT_DISTURB: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::DND);
			_dnd_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "dnd");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_AWAY_TEMPORARILY: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::Away);
			_away_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "away");
			BlabberSettings::Instance()->WriteToFile();
			
			break;
		}

		case BLAB_AWAY_EXTENDED: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::XA);
			_xa_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "xa");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_SCHOOL: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::XA, "Off to School");
			_school_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "school");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_WORK: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::XA, "Busy at Work");
			_work_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "work");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_LUNCH: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::Away, "Lunch");
			_lunch_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "lunch");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_DINNER: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::Away, "Dinner");
			_dinner_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "dinner");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_SLEEP: {
			JabberSpeak::Instance()->SendPresence(gloox::Presence::XA, "Sleeping");
			_sleep_item->SetMarked(true);
			BlabberSettings::Instance()->SetTag("last-used-custom-status", false);
			BlabberSettings::Instance()->SetData("last-custom-exact-status", "sleep");
			BlabberSettings::Instance()->WriteToFile();

			break;
		}

		case BLAB_CUSTOM_STATUS:
		{
			CustomStatusWindow::Instance()->Show();
			CustomStatusWindow::Instance()->Activate();
			break;
		}
		
		case kResetWindow:
		{
			JabberSpeak::Instance()->Reset();
			ShowLogin();
			
			break;
		}

		case JAB_ROTATE_CHAT_FORWARD:
		{
			int currentIndex = fTalkCards->VisibleIndex();
			int itemCount = fTalkCards->CountItems();

			if (++currentIndex >= itemCount)
				currentIndex = 0;

			if (currentIndex < itemCount) {
				fTalkCards->SetVisibleItem(currentIndex);
				TalkView* v = (TalkView*)fTalkCards->ItemAt(currentIndex)->View();
				if (v->IsGroupChat()) {
					int item = _roster->FindBookmark(gloox::JID(v->GetGroupRoom()));
					_roster->Select(item);
				} else {
					int item = _roster->FindUser(v->GetUserID());
					_roster->Select(item);
				}
			}
			break;
		}

		case JAB_ROTATE_CHAT_BACKWARD:
		{
			int currentIndex = fTalkCards->VisibleIndex();
			int itemCount = fTalkCards->CountItems();

			if (--currentIndex < 0)
				currentIndex = itemCount - 1;

			if (currentIndex > 0) {
				fTalkCards->SetVisibleItem(currentIndex);
				TalkView* v = (TalkView*)fTalkCards->ItemAt(currentIndex)->View();
				if (v->IsGroupChat()) {
					int item = _roster->FindBookmark(gloox::JID(v->GetGroupRoom()));
					_roster->Select(item);
				} else {
					int item = _roster->FindUser(v->GetUserID());
					_roster->Select(item);
				}
			}
			break;
		}

		case B_OBSERVER_NOTICE_CHANGE:
		{
			int32 orig_what = msg->FindInt32("be:observe_change_what");
			switch (orig_what) {
				case kAuthenticationFailed:
				{
					ShowLogin();
					_login_username->MarkAsInvalid(true);
					_login_password->MarkAsInvalid(true);
					break;
				}
			}

			break;
		}
		
		break;
	}
}

void BlabberMainWindow::MenusBeginning() {
	char buffer[1024];

	// FILE menu
	if (!_full_view->IsHidden()) {
		_connect_item->SetEnabled(false);
		_disconnect_item->SetEnabled(true);
	} else {
		_connect_item->SetEnabled(true);
		_disconnect_item->SetEnabled(false);
	}

	// logging
	bool isGroup = true;
	bool isLogging = false;
	TalkView* talk = nullptr;

	BLayoutItem* i = fTalkCards->VisibleItem();
	if (i) {
		talk = (TalkView*)i->View();
		isGroup = talk->IsGroupChat();
		isLogging = talk->IsLogging();
	}

	if(_record_item != nullptr) {
	  _record_item->SetEnabled(!isGroup && !isLogging);
	  _record_item->SetTarget(talk);
	}
	_record_entire_item->SetEnabled(!isGroup && isLogging);
	_record_entire_item->SetTarget(talk);

	// EDIT menu
	if (RosterItem *item = _roster->CurrentItemSelection()) {
		// if a  item is selected
		sprintf(buffer, "Edit %s", item->GetUserID()->FriendlyName().c_str());
		_change_buddy_item->SetLabel(buffer);
		_change_buddy_item->SetEnabled(true);

		sprintf(buffer, "Remove %s", item->GetUserID()->FriendlyName().c_str());
		_remove_buddy_item->SetLabel(buffer);
		_remove_buddy_item->SetEnabled(true);

		_user_info_item->SetEnabled(true);
		_user_chatlog_item->SetEnabled(BlabberSettings::Instance()->Tag("autoopen-chatlog"));
	} else {
		sprintf(buffer, "Edit Buddy");
		_change_buddy_item->SetLabel(buffer);
		_change_buddy_item->SetEnabled(false);

		sprintf(buffer, "Remove Buddy");
		_remove_buddy_item->SetLabel(buffer);
		_remove_buddy_item->SetEnabled(false);

		_user_info_item->SetEnabled(false);
		_user_chatlog_item->SetEnabled(false);
	}

	const char *message_1 = BlabberSettings::Instance()->Data("message-1");
	const char *message_2 = BlabberSettings::Instance()->Data("message-2");
	const char *message_3 = BlabberSettings::Instance()->Data("message-3");
	const char *message_4 = BlabberSettings::Instance()->Data("message-4");
	const char *message_5 = BlabberSettings::Instance()->Data("message-5");
	const char *message_6 = BlabberSettings::Instance()->Data("message-6");
	const char *message_7 = BlabberSettings::Instance()->Data("message-7");
	const char *message_8 = BlabberSettings::Instance()->Data("message-8");
	const char *message_9 = BlabberSettings::Instance()->Data("message-9");
	
	if (message_1) {
		_message_1_item->SetLabel(message_1);
		_message_1_item->SetEnabled(true);
	} else {
		_message_1_item->SetLabel("Message 1");
		_message_1_item->SetEnabled(false);
	}

	if (message_2) {
		_message_2_item->SetLabel(message_2);
		_message_2_item->SetEnabled(true);
	} else {
		_message_2_item->SetLabel("Message 2");
		_message_2_item->SetEnabled(false);
	}

	if (message_3) {
		_message_3_item->SetLabel(message_3);
		_message_3_item->SetEnabled(true);
	} else {
		_message_3_item->SetLabel("Message 3");
		_message_3_item->SetEnabled(false);
	}

	if (message_4) {
		_message_4_item->SetLabel(message_4);
		_message_4_item->SetEnabled(true);
	} else {
		_message_4_item->SetLabel("Message 4");
		_message_4_item->SetEnabled(false);
	}

	if (message_5) {
		_message_5_item->SetLabel(message_5);
		_message_5_item->SetEnabled(true);
	} else {
		_message_5_item->SetLabel("Message 5");
		_message_5_item->SetEnabled(false);
	}

	if (message_6) {
		_message_6_item->SetLabel(message_6);
		_message_6_item->SetEnabled(true);
	} else {
		_message_6_item->SetLabel("Message 6");
		_message_6_item->SetEnabled(false);
	}

	if (message_7) {
		_message_7_item->SetLabel(message_7);
		_message_7_item->SetEnabled(true);
	} else {
		_message_7_item->SetLabel("Message 7");
		_message_7_item->SetEnabled(false);
	}

	if (message_8) {
		_message_8_item->SetLabel(message_8);
		_message_8_item->SetEnabled(true);
	} else {
		_message_8_item->SetLabel("Message 8");
		_message_8_item->SetEnabled(false);
	}

	if (message_9) {
		_message_9_item->SetLabel(message_9);
		_message_9_item->SetEnabled(true);
	} else {
		_message_9_item->SetLabel("Message 9");
		_message_9_item->SetEnabled(false);
	}

	// FIXME do this when swapping talkcards instead
	if (fTalkCards->VisibleItem()) {
		_message_menu->SetTargetForItems(fTalkCards->VisibleItem()->View());
		_message_menu->SetEnabled(true);
	} else {
		_message_menu->SetEnabled(false);
	}
}

bool BlabberMainWindow::QuitRequested() {
	// remember last coordinates
	BlabberSettings::Instance()->SetFloatData("main-window-left", Frame().left);
	BlabberSettings::Instance()->SetFloatData("main-window-top", Frame().top);
	BlabberSettings::Instance()->SetFloatData("main-window-width", Bounds().Width());
	BlabberSettings::Instance()->SetFloatData("main-window-height", Bounds().Height());
	BlabberSettings::Instance()->WriteToFile();

	// FIXME we are doing this from the wrong thread, but I see no better place.
	// It needs to be done immediately here, so that closing the other windows
	// doen't remove them from autojoin.
	BookmarkManager::Instance().Disconnect();

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

BlabberMainWindow::BlabberMainWindow(BRect frame)
	: BWindow(frame, "Renga", B_DOCUMENT_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS)
	, _record_item(nullptr)
{

	// editing filter for taksing
	AddCommonFilter(new RotateChatFilter());
	
	// add self to message family
	MessageRepeater::Instance()->AddTarget(this);

	// encompassing view
	_full_view = new BGroupView("main-full", B_VERTICAL);
	_full_view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	// status bar
	_status_view = new StatusView();
	_status_view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	
	// menubar
	BMenuBar* menubar = new BMenuBar("menubar");

	// FILE MENU
	BMenu* file_menu = new BMenu("File");

		_connect_item    = new BMenuItem("Log On", new BMessage(JAB_CONNECT));
		_connect_item->SetShortcut('N', 0);

		_disconnect_item = new BMenuItem("Log Off", new BMessage(JAB_DISCONNECT));
		_disconnect_item->SetShortcut('B', 0);

		_about_item      = new BMenuItem("About Renga" B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED));

		_quit_item = new BMenuItem("Quit", new BMessage(JAB_QUIT));
		_quit_item->SetShortcut('Q', 0);

    bool bAutoOpenChatLog = BlabberSettings::Instance()->Tag("autoopen-chatlog");
	if(bAutoOpenChatLog) {
		BMessage *msg = new BMessage(JAB_SHOW_CHATLOG);
		_record_entire_item = new BMenuItem("Show Chat Log", msg);
		_record_entire_item->SetShortcut('H', 0);
	} else {
		_record_item = new BMenuItem("Start Chat Log", new BMessage(JAB_START_RECORD));
		_record_entire_item = new BMenuItem("Stop Chat Log", new BMessage(JAB_STOP_RECORD));
	}

	if(_record_item != nullptr) {
	  file_menu->AddItem(_record_item);
	}
	file_menu->AddItem(_record_entire_item);
	file_menu->AddSeparatorItem();

//	file_menu->AddItem(_connect_item);
	file_menu->AddItem(_disconnect_item);
	file_menu->AddSeparatorItem();
	file_menu->AddItem(_quit_item);
	file_menu->SetTargetForItems(MessageRepeater::Instance());

	// EDIT MENU
	BMenu* edit_menu = new BMenu("Edit");

		_add_buddy_item = new BMenuItem("Add New Buddy", new BMessage(JAB_OPEN_ADD_BUDDY_WINDOW));
		_add_buddy_item->SetShortcut('N', 0);

		_change_buddy_item = new BMenuItem("Edit Buddy", new BMessage(JAB_OPEN_EDIT_BUDDY_WINDOW));
		_change_buddy_item->SetShortcut('E', 0);

		_remove_buddy_item = new BMenuItem("Remove Buddy", new BMessage(JAB_REMOVE_BUDDY));
		_remove_buddy_item->SetShortcut('T', 0);

		_user_info_item = new BMenuItem("Get User Info", new BMessage(JAB_USER_INFO));
		_user_info_item->SetShortcut('I', 0);
		
		_user_chatlog_item = new BMenuItem("Show Chat Log", new BMessage(JAB_SHOW_CHATLOG));
		_user_chatlog_item->SetShortcut('H', 0);
		
		_preferences_item = new BMenuItem("Preferences...", new BMessage(JAB_PREFERENCES));

	edit_menu->AddItem(_add_buddy_item);
	edit_menu->AddItem(_change_buddy_item);
	edit_menu->AddItem(_remove_buddy_item);
	edit_menu->AddSeparatorItem();
	edit_menu->AddItem(_user_info_item);
	edit_menu->AddSeparatorItem();
	edit_menu->AddItem(_user_chatlog_item);
	edit_menu->AddSeparatorItem();
	edit_menu->AddItem(_preferences_item);
	edit_menu->SetTargetForItems(this);

	// STATUS MENU
	BMenu* status_menu = new BMenu("Status");
	
		_chat_item = new BMenuItem("I'm available for chat.", new BMessage(BLAB_AVAILABLE_FOR_CHAT));
		_away_item = new BMenuItem("I will be away temporarily.", new BMessage(BLAB_AWAY_TEMPORARILY));
		_dnd_item = new BMenuItem("Do not disturb me.", new BMessage(BLAB_DO_NOT_DISTURB));
		_xa_item = new BMenuItem("I will be away for an extended time period.", new BMessage(BLAB_AWAY_EXTENDED));
		_school_item = new BMenuItem("Off to School", new BMessage(BLAB_SCHOOL));
		_work_item = new BMenuItem("Busy at Work", new BMessage(BLAB_WORK));
		_lunch_item = new BMenuItem("Lunch", new BMessage(BLAB_LUNCH));
		_dinner_item = new BMenuItem("Dinner", new BMessage(BLAB_DINNER));
		_sleep_item = new BMenuItem("Sleeping", new BMessage(BLAB_SLEEP));
		_custom_item = new BMenuItem("Custom...", new BMessage(BLAB_CUSTOM_STATUS));

	status_menu->AddItem(_chat_item);
	status_menu->AddSeparatorItem();
	status_menu->AddItem(_away_item);
	status_menu->AddItem(_dnd_item);
	status_menu->AddItem(_xa_item);
	status_menu->AddSeparatorItem();

	status_menu->AddItem(_school_item);
	status_menu->AddItem(_work_item);
	status_menu->AddItem(_lunch_item);
	status_menu->AddItem(_dinner_item);
	status_menu->AddItem(_sleep_item);

	status_menu->AddSeparatorItem();
	status_menu->AddItem(_custom_item);

	status_menu->SetRadioMode(true);
	_chat_item->SetMarked(true);
	
	// TALK MENU
	BMenu* talk_menu = new BMenu("Talk");

		BMenuItem* rotate_chat_forward_item = new BMenuItem("Rotate Chat Forward", new BMessage(JAB_ROTATE_CHAT_FORWARD));
		rotate_chat_forward_item->SetShortcut('.', 0);

		BMenuItem* rotate_chat_backward_item = new BMenuItem("Rotate Chat Backward", new BMessage(JAB_ROTATE_CHAT_BACKWARD));
		rotate_chat_backward_item->SetShortcut(',', 0);

		_send_message_item = new BMenuItem("Send Message...", new BMessage(JAB_OPEN_NEW_MESSAGE));
		_send_message_item->SetShortcut('M', 0);

		_send_chat_item = new BMenuItem("Start Chat...", new BMessage(JAB_OPEN_NEW_CHAT));
		_send_chat_item->SetShortcut('C', 0);

		_send_groupchat_item = new BMenuItem("Start Group Chat...", new BMessage(JAB_OPEN_NEW_GROUP_CHAT));
		_send_groupchat_item->SetShortcut('G', 0);

	talk_menu->AddItem(rotate_chat_forward_item);
	talk_menu->AddItem(rotate_chat_backward_item);
	talk_menu->AddSeparatorItem();
	talk_menu->AddItem(_send_message_item);
	talk_menu->AddItem(_send_chat_item);
	talk_menu->AddItem(_send_groupchat_item);
	talk_menu->SetTargetForItems(this);

	// MESSAGE MENU
	_message_menu = new BMenu("Messages");

		_message_1_item = new BMenuItem("Message #1", new BMessage(JAB_MESSAGE_1));
		_message_1_item->SetShortcut('1', 0);
		_message_1_item->SetEnabled(false);

		_message_2_item = new BMenuItem("Message #2", new BMessage(JAB_MESSAGE_2));
		_message_2_item->SetShortcut('2', 0);
		_message_2_item->SetEnabled(false);

		_message_3_item = new BMenuItem("Message #3", new BMessage(JAB_MESSAGE_3));
		_message_3_item->SetShortcut('3', 0);
		_message_3_item->SetEnabled(false);

		_message_4_item = new BMenuItem("Message #4", new BMessage(JAB_MESSAGE_4));
		_message_4_item->SetShortcut('4', 0);
		_message_4_item->SetEnabled(false);

		_message_5_item = new BMenuItem("Message #5", new BMessage(JAB_MESSAGE_5));
		_message_5_item->SetShortcut('5', 0);
		_message_5_item->SetEnabled(false);

		_message_6_item = new BMenuItem("Message #6", new BMessage(JAB_MESSAGE_6));
		_message_6_item->SetShortcut('6', 0);
		_message_6_item->SetEnabled(false);

		_message_7_item = new BMenuItem("Message #7", new BMessage(JAB_MESSAGE_7));
		_message_7_item->SetShortcut('7', 0);
		_message_7_item->SetEnabled(false);

		_message_8_item = new BMenuItem("Message #8", new BMessage(JAB_MESSAGE_8));
		_message_8_item->SetShortcut('8', 0);
		_message_8_item->SetEnabled(false);
		
		_message_9_item = new BMenuItem("Message #9", new BMessage(JAB_MESSAGE_9));
		_message_9_item->SetShortcut('9', 0);
		_message_9_item->SetEnabled(false);

	_message_menu->AddItem(_message_1_item);
	_message_menu->AddItem(_message_2_item);
	_message_menu->AddItem(_message_3_item);
	_message_menu->AddItem(_message_4_item);
	_message_menu->AddItem(_message_5_item);
	_message_menu->AddItem(_message_6_item);
	_message_menu->AddItem(_message_7_item);
	_message_menu->AddItem(_message_8_item);
	_message_menu->AddItem(_message_9_item);

	// HELP MENU
	BMenu* help_menu = new BMenu("Help");

	BMenuItem* user_guide_item = new BMenuItem("Renga Manual", new BMessage(JAB_USER_GUIDE));
	BMenuItem* faq_item = new BMenuItem("Renga FAQ", new BMessage(JAB_FAQ));

	help_menu->AddItem(user_guide_item);
	help_menu->AddItem(faq_item);
	help_menu->AddSeparatorItem();
	help_menu->AddItem(_about_item);

	help_menu->SetTargetForItems(this);

	menubar->AddItem(file_menu);
	menubar->AddItem(edit_menu);
	menubar->AddItem(status_menu);
	menubar->AddItem(talk_menu);
	menubar->AddItem(_message_menu);
	menubar->AddItem(help_menu);

	// tabbed view
	// roster view
	_roster          = new RosterView();
	BScrollView* roster_scroller = new BScrollView(NULL, _roster, 0, false, true,
		B_NO_BORDER);
	_roster->TargetedByScrollView(roster_scroller);

	// chat service
	BLayoutBuilder::Group<>(_full_view, B_VERTICAL, 0)
		.SetInsets(0, 0, 0, 0)
		.Add(menubar)
		.AddSplit(B_HORIZONTAL, 0)
			.AddGroup(B_VERTICAL, 0, 0)
				.Add(roster_scroller)
				.Add(_status_view)
			.End()
			.AddCards()
				.GetLayout(&fTalkCards)
			.End()
		.End()
	.End();
	
	///// NOW DO LOGIN STUFF
	// encompassing view
	_login_full_view = new BGroupView("login-full", B_VERTICAL);
	_login_full_view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	// graphics
	_login_bulb = new PictureView("bulb-normal");

	// username/password controls
	_login_realname = new BTextControl(NULL, "Nickname: ", NULL, NULL);
	
	_login_username = new BTextControl(NULL, "Jabber ID: ", NULL, NULL);
		
	_login_password = new BTextControl(NULL, "Password: ", NULL, NULL);
	_login_password->TextView()->HideTyping(true);
	
	BMessage* createAccount = new BMessage(kCreateAccount);
	_login_new_account = new BButton("create", "Create a new account",
		createAccount);

	_login_auto_login = new BCheckBox(NULL, "Auto-login", NULL);

	// login button
	_login_login = new BButton("login", "Login", new BMessage(JAB_LOGIN));
	_login_login->MakeDefault(false);
	_login_login->SetTarget(this);

	BStringView* appName = new BStringView("app name", "Renga");
	BFont bigfont(be_plain_font);
	bigfont.SetSize(bigfont.Size() * 3);
	appName->SetFont(&bigfont);

	BLayoutBuilder::Group<>(_login_full_view, B_HORIZONTAL)
		.AddGlue()
		.AddGroup(B_VERTICAL)
			.SetInsets(B_USE_DEFAULT_SPACING)
			.AddGrid()
				.Add(_login_bulb, 0, 0, 1, 4)
				.Add(appName, 1, 1)
				.Add(new BStringView("", "XMPP client for Haiku"), 1, 2)
			.End()
			.Add(_login_new_account)
			.AddGlue()
			.AddGrid()
				.AddTextControl(_login_realname, 0, 0)
				.AddTextControl(_login_username, 0, 1)
				.AddTextControl(_login_password, 0, 2)
			.End()
			.AddGroup(B_HORIZONTAL)
				.Add(_login_auto_login)
				.AddGlue()
				.Add(_login_login)
			.End()
			.AddGlue()
		.End()
		.AddGlue()
	.End();
	
	// attach all-encompassing main view to window
	BCardLayout* cl = new BCardLayout();
	SetLayout(cl);
	AddChild(_full_view);
	AddChild(_login_full_view);

	// login always hidden at start
	cl->SetVisibleItem((int32)0);

	// default
	if(BlabberSettings::Instance()->Data("last-realname")) {
		_login_realname->SetText(BlabberSettings::Instance()->Data("last-realname"));
	} else {
		_login_realname->SetText("Me");
	}
	
	if (BlabberSettings::Instance()->Data("last-login")) {
		_login_username->SetText(BlabberSettings::Instance()->Data("last-login"));
	} else {
		_login_username->SetText("newuser@jabber.org");
	}
	
	_login_password->SetText(BlabberSettings::Instance()->Data("last-password"));

	_login_auto_login->SetValue(BlabberSettings::Instance()->Tag("auto-login"));

	// focus to start
	if (BlabberSettings::Instance()->Data("last-login")) {
		_login_password->MakeFocus(true);
	} else {
		_login_username->MakeFocus(true);
	}

	JabberSpeak::Instance()->StartWatchingAll(this);

	// editing filter for taksing
	AddCommonFilter(new RotateChatFilter());
}


bool BlabberMainWindow::ValidateLogin() {
	// existance of username
	if (!strcmp(_login_username->Text(), "")) {
		ModalAlertFactory::Alert("Wait, you haven't specified your Jabber ID yet.\n(e.g. haikuFan@jabber.org)", "Doh!", NULL, NULL, B_WIDTH_FROM_LABEL, B_STOP_ALERT);
		_login_username->MakeFocus(true);

		return false;
	}

	// validity of username
	std::string validate = UserID::WhyNotValidJabberHandle(_login_username->Text());
	if (validate.size()) {
		BString buffer;
		buffer.SetToFormat("The Jabber ID you specified must not be yours because it's invalid for the following reason:\n\n%s\n\nIf you can't remember it, it's OK to create a new one by checking the \"Create a new Jabber Account!\" box.", validate.c_str());

		ModalAlertFactory::Alert(buffer, "OK", NULL, NULL,
			B_WIDTH_FROM_LABEL, B_STOP_ALERT);
		_login_username->MakeFocus(true);

		return false;
	}

	gloox::JID username(_login_username->Text());

	// check existance of password
	if (!strcmp(_login_password->Text(), "")) {
		char buffer[1024];
		sprintf(buffer, "You must specify a password so I can make sure it's you, %s.",
			username.username().c_str());

		ModalAlertFactory::Alert(buffer, "Sorry!", NULL, NULL,
			B_WIDTH_FROM_LABEL, B_STOP_ALERT);
		_login_password->MakeFocus(true);

		return false;
	}

	return true;
}

void BlabberMainWindow::ShowLogin() {
	// reassign button as default
	_login_login->MakeDefault(true);

	// reset
	_login_realname->SetText("");
	_login_username->SetText("");
	_login_password->SetText("");

	_login_username->MarkAsInvalid(false);
	_login_password->MarkAsInvalid(false);

	// default
	if(BlabberSettings::Instance()->Data("last-realname")) {
		_login_realname->SetText(BlabberSettings::Instance()->Data("last-realname"));
	} else {
		_login_realname->SetText("Me");
	}
	
	if (BlabberSettings::Instance()->Data("last-login")) {
		_login_username->SetText(BlabberSettings::Instance()->Data("last-login"));
	} else {
		_login_username->SetText("newuser@jabber.org");
	}

	_login_password->SetText(BlabberSettings::Instance()->Data("last-password"));

	// focus to start
	if (BlabberSettings::Instance()->Data("last-login")) {
		_login_password->MakeFocus(true);
	} else {
		_login_username->MakeFocus(true);
	}

	BCardLayout* cl = (BCardLayout*)GetLayout();
	cl->SetVisibleItem(1);
}

void BlabberMainWindow::SetCustomStatus(string status) {
	char buffer[2048];
	
	// create menued status message
	sprintf(buffer, "[Custom] %s", status.c_str()); 

	_custom_item->SetMarked(true);
	_custom_item->SetLabel(buffer);
}


void
BlabberMainWindow::AddTalkView(TalkView* view)
{
	if (view->Window() != this) {
		fTalkCards->AddView(view);
	}

	fTalkCards->SetVisibleItem(fTalkCards->IndexOfView(view));
}
