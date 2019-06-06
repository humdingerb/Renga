//////////////////////////////////////////////////
// Blabber [TalkManager.cpp]
//////////////////////////////////////////////////

#ifndef __CSTDIO__
	#include <cstdio>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

#ifndef BLABBER_SETTINGS_H
	#include "BlabberSettings.h"
#endif

#ifndef JABBER_SPEAK_H
	#include "JabberSpeak.h"
#endif

#ifndef MESSAGE_REPEATER_H
	#include "MessageRepeater.h"
#endif

#ifndef MESSAGES_H
	#include "Messages.h"
#endif

#ifndef MODAL_ALERT_FACTORY_H
	#include "ModalAlertFactory.h"
#endif

#ifndef SOUND_SYSTEM_H
	#include "SoundSystem.h"
#endif

#ifndef TALK_MANAGER_H
	#include "TalkManager.h"
#endif

TalkManager *TalkManager::_instance = NULL;

TalkManager *TalkManager::Instance() {
	if (_instance == NULL) {
		_instance = new TalkManager();
	}

	return _instance;
}

TalkManager::TalkManager() {
}

TalkManager::~TalkManager() {
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		i->second->PostMessage(B_QUIT_REQUESTED);
	}

	_instance = NULL;
}

TalkWindow *TalkManager::CreateTalkSession(const TalkWindow::talk_type type, const UserID *user, string group_room, string group_username, string thread, bool sound_on_new) {
	
	TalkWindow *window = NULL;
	
	// is there a window already?
	if (type != TalkWindow::GROUP && IsExistingWindowToUser(type, user->Handle()).size()) {
		window = _talk_map[IsExistingWindowToUser(type, user->Handle())];

		// activate it
		if (!sound_on_new) {
			window->Activate();
		}
	} else if (type != TalkWindow::GROUP) {
		// create a new window
		if (sound_on_new) {
			window = new TalkWindow(type, user, "", "", true);
		} else {
			window = new TalkWindow(type, user, "", "");
		}
		
		window->SetThreadID(thread);

		if (sound_on_new) {
			// play a sound
			SoundSystem::Instance()->PlayNewMessageSound();
		}
		
		// add it to the known list BUGBUG we need to remove this when window closes
		_talk_map[thread] = window;
	} else if (type == TalkWindow::GROUP && IsExistingWindowToGroup(type, group_room).size()) {
		window = _talk_map[IsExistingWindowToGroup(type, group_room)];

		// activate it
		if (!sound_on_new) {
			window->Activate();
		}
	} else if (type == TalkWindow::GROUP) {
		// create a new window
		if (sound_on_new) {
			window = new TalkWindow(type, user, group_room, group_username, true);
		} else {
			window = new TalkWindow(type, user, group_room, group_username);
		}
		
		window->SetThreadID(thread);

		// add it to the known list BUGBUG we need to remove this when window closes
		_talk_map[thread] = window;

		// send presence
		JabberSpeak::Instance()->SendGroupPresence(group_room, group_username);
	}
	
	// return a reference as well
	return window;
}

void
TalkManager::handleMessage(const gloox::Message &msg, __attribute__((unused)) gloox::MessageSession *session)
{
	TalkWindow::talk_type type;
	string                thread_id;
	string                sender;
	TalkWindow           *window = NULL;

	string                group_room;
	string                group_server;
	string                group_identity;
	string                group_username;

	// must be content to continue
	if (msg.body() == "") {
		return;
	}

	// configure type
	if (msg.subtype() == gloox::Message::Normal) {
		if (BlabberSettings::Instance()->Tag("convert-messages-to-chat")) {
			type = TalkWindow::CHAT;
		} else {
			type = TalkWindow::MESSAGE;
		}
	} else if (msg.subtype() == gloox::Message::Chat) {
		type = TalkWindow::CHAT;
	} else if (msg.subtype() == gloox::Message::Groupchat) {
		type = TalkWindow::GROUP;
	} else if (msg.subtype() == gloox::Message::Error) {
		char buffer[2048];
		
		sprintf(buffer, "An error occurred when you tried sending a message to %s.  The reason is as follows:\n\n%s", msg.from().full().c_str(), msg.body().c_str());
		ModalAlertFactory::Alert(buffer, "Oh, well.", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		
		return;
	} else {
		// ignore other messages
		return;
	}

	// configure sender
	sender = msg.from().full();

	// configure thread ID
	thread_id = msg.thread();

	if (type == TalkWindow::CHAT && _talk_map.count(thread_id) == 0) {
		// is there a window with the same sender already open? (only for chat)
		if (!IsExistingWindowToUser(type, sender).empty()) {
			thread_id = IsExistingWindowToUser(type, sender);
		}
	} else if (type == TalkWindow::GROUP) {
		// separate room and server
		string::size_type at_pos = sender.find("@");

		if (at_pos != string::npos) {
			// get group room
			group_room = sender.substr(0, at_pos);
			
			// clear out text
			sender = sender.substr(at_pos + 1);
			
			// now pare out server and username
			string::size_type slash_pos = sender.find("/");
			
			if (slash_pos != string::npos) {
				// get server
				group_server = sender.substr(0, slash_pos);

				// clear out text
				group_username = sender.substr(slash_pos + 1);

				// create identity
				group_identity = group_room + '@' + group_server;
			} else {
				// get server
				group_server = sender.substr(0, slash_pos);

				// create identity
				group_identity = group_room + '@' + group_server;
			}
		} else {
			return;
		}
		
		// is there a window with the same sender already open? (only for chat)
		if (!IsExistingWindowToGroup(type, group_identity).empty()) {
			thread_id = IsExistingWindowToGroup(type, group_identity);
		}
	}

	// create new thread ID
	if (thread_id.empty() || _talk_map.count(thread_id) == 0) {
		if (type == TalkWindow::GROUP) {
			return;
		}

		thread_id = GenericFunctions::GenerateUniqueID();

		// configure user ID
		UserID *user;

		JRoster::Instance()->Lock();

		if (JRoster::Instance()->FindUser(JRoster::HANDLE, UserID(sender).JabberHandle())) {
			UserID *tmp_user = JRoster::Instance()->FindUser(JRoster::HANDLE, UserID(sender).JabberHandle());
			user = new UserID(*tmp_user);
		} else {
			user = new UserID(sender);

			if (user->UserType() == UserID::INVALID) {
				user->SetFriendlyName("Jabber Server Host");
			}
		}

		JRoster::Instance()->Unlock();

		// create the new session								
		window = CreateTalkSession(type, user, group_room, group_username, thread_id, true);
	} else {
		window = _talk_map[thread_id];
	}

	// submit the chat
	if (window) {
		window->Lock();

		if (type == TalkWindow::GROUP) {
			if (group_username.empty()) {
				window->AddToTalk("System:", msg.body(), TalkWindow::OTHER);
			} else {
				window->NewMessage(group_username, msg.body());
			}
		} else {
			window->NewMessage(msg.body());
		}
		window->Unlock();
	}
}

string TalkManager::IsExistingWindowToUser(TalkWindow::talk_type type, string username) {
	// check handles (with resource)
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		if ((*i).second->Type() == type && (*i).second->GetUserID()->Handle() == UserID(username).Handle()) {
			return (*i).first;
		}
	}

	// check handles (without resource)
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		if ((*i).second->Type() == type && (*i).second->GetUserID()->JabberHandle() == UserID(username).JabberHandle()) {
			return (*i).first;
		}
	}

	// no matches
	return "";
}

string TalkManager::IsExistingWindowToGroup(TalkWindow::talk_type type, string group_room) {
	// check names
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		if ((*i).second->Type() == type && (*i).second->GetGroupRoom() == group_room) {
			return (*i).first;
		}
	}

	// no matches
	return "";
}

void TalkManager::UpdateWindowTitles(const UserID *user) {
	// check handles (without resource)
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		if ((*i).second->GetUserID()->JabberHandle() == user->JabberHandle()) {
			(*i).second->SetTitle(user->FriendlyName().c_str());
		}
	}
}

void TalkManager::RemoveWindow(string thread_id) {
	if (_talk_map.count(thread_id) > 0) {
		_talk_map.erase(thread_id);
	}
}

void TalkManager::RotateToNextWindow(TalkWindow *current, rotation direction) {
	// no chat windows
	if (_talk_map.size() == 0) {
		return;
	}

	// from chat windows
	if (_talk_map.size() == 1 && current != NULL) {
		return;
	}

	// remember first and last, we may need them later
	TalkWindow *first = (*_talk_map.begin()).second;
	TalkWindow *last  = (*_talk_map.rbegin()).second;
		
	// from non-chat windows
	if (current == NULL) {
		if (direction == ROTATE_FORWARD) {
			first->Activate();
		} else {
			last->Activate();
		}
		
		return;
	}
	
	// iterate and find the current window
	TalkWindow *previous = NULL;
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		if ((*i).second == current) {
			// we found our window, now check bordering windows
			if (direction == ROTATE_FORWARD) {
				if (++i != _talk_map.end()) {
					(*i).second->Activate();
				} else {
					first->Activate();
				}
			} else {
				if (previous) {
					previous->Activate();
				} else {
					last->Activate();
				}
			}

			break;
		} else {
			previous = (*i).second;
		}
	}
}

void TalkManager::Reset() {
	MessageRepeater::Instance()->PostMessage(JAB_CLOSE_TALKS);
	_talk_map.clear();
}
