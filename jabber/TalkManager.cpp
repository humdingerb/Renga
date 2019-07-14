//////////////////////////////////////////////////
// Blabber [TalkManager.cpp]
//////////////////////////////////////////////////

#include <gloox/instantmucroom.h>

#include <cstdio>

#include <interface/Window.h>

#include "BlabberSettings.h"
#include "JabberSpeak.h"
#include "MessageRepeater.h"
#include "Messages.h"
#include "ModalAlertFactory.h"
#include "SoundSystem.h"
#include "TalkManager.h"

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

TalkWindow *TalkManager::CreateTalkSession(const gloox::Message::MessageType type,
	const UserID* user, string group_room, string group_username,
	string thread, bool sound_on_new)
{
	TalkWindow *window = NULL;
	
	// is there a window already?
	if (type != gloox::Message::Groupchat && IsExistingWindowToUser(type, user->Handle()).size()) {
		window = _talk_map[IsExistingWindowToUser(type, user->Handle())];

		// activate it
		if (!sound_on_new) {
			window->Activate();
		}
	} else if (type != gloox::Message::Groupchat) {
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
	} else if (type == gloox::Message::Groupchat && IsExistingWindowToGroup(type, group_room).size()) {
		window = _talk_map[IsExistingWindowToGroup(type, group_room)];

		// activate it
		if (!sound_on_new) {
			window->Activate();
		}
	} else if (type == gloox::Message::Groupchat) {
		// create a new window
		if (sound_on_new) {
			window = new TalkWindow(type, user, group_room, group_username, true);
		} else {
			window = new TalkWindow(type, user, group_room, group_username);
		}
		
		window->SetThreadID(thread);

		// add it to the known list BUGBUG we need to remove this when window closes
		_talk_map[thread] = window;

		// FIXME we need to free this when leaving the room!
		gloox::JID jid(group_room);
		jid.setResource(group_username);
		fprintf(stderr, "TRY TO JOIN MUC %s\n", group_room.c_str());
		(new gloox::InstantMUCRoom(JabberSpeak::Instance()->GlooxClient(),
			jid, this))->join();
	}
	
	// return a reference as well
	return window;
}

void
TalkManager::handleMessage(const gloox::Message &msg,
	__attribute__((unused)) gloox::MessageSession *session)
{
	gloox::Message::MessageType type;
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
	type = msg.subtype();
	if (type == gloox::Message::Normal) {
		if (BlabberSettings::Instance()->Tag("convert-messages-to-chat"))
			type = gloox::Message::Chat;
	} else if (type == gloox::Message::Error) {
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

	if (type == gloox::Message::Chat && _talk_map.count(thread_id) == 0) {
		// is there a window with the same sender already open? (only for chat)
		if (!IsExistingWindowToUser(type, sender).empty()) {
			thread_id = IsExistingWindowToUser(type, sender);
		}
	} else if (type == gloox::Message::Groupchat) {
		fprintf(stderr, "received group message at %s\n", __PRETTY_FUNCTION__);
	}

	// create new thread ID
	if (thread_id.empty() || _talk_map.count(thread_id) == 0) {
		if (type == gloox::Message::Groupchat) {
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

		if (type == gloox::Message::Groupchat) {
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

string
TalkManager::IsExistingWindowToUser(gloox::Message::MessageType type,
	string username)
{
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

string TalkManager::IsExistingWindowToGroup(gloox::Message::MessageType type, string group_room) {
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


void
TalkManager::handleMUCParticipantPresence(gloox::MUCRoom *room,
							const gloox::MUCRoomParticipant participant,
							const gloox::Presence &presence)
{
	BMessage msg;
	BString fullRoom;
	fullRoom.SetToFormat("%s@%s", room->name().c_str(), room->service().c_str());
	msg.AddString("room", fullRoom);
	msg.AddString("server", room->service().c_str());
	msg.AddString("username", participant.nick->resource().c_str());
	if (presence.subtype() == gloox::Presence::Available) {
		msg.what = JAB_GROUP_CHATTER_ONLINE;
	} else if (presence.subtype() == gloox::Presence::Unavailable) {
		msg.what = JAB_GROUP_CHATTER_OFFLINE;
	}
	MessageRepeater::Instance()->PostMessage(&msg);
}


void
TalkManager::handleMUCMessage(gloox::MUCRoom *room,
	const gloox::Message &msg, bool priv __attribute__((unused)))
{
	string thread_id;
	TalkWindow *window = NULL;

	string group_room;
	string group_server;
	string group_username;
	string group_identity;

	// get group room
	group_room = room->name();
			
	// clear out text
	group_username = msg.from().resource();
			
	// get server
	group_server = room->service();

	// create identity
	group_identity = group_room + '@' + group_server;
		
	// is there a window with the same sender already open? (only for chat)
	if (!IsExistingWindowToGroup(gloox::Message::Groupchat, group_identity).empty()) {
		thread_id = IsExistingWindowToGroup(gloox::Message::Groupchat, group_identity);
	} else {
		fprintf(stderr, "Failed to find chat window\n");
		return;
	}
	window = _talk_map[thread_id];
	// submit the chat
	if (window) {
		window->Lock();

		if (group_username.empty()) {
			window->AddToTalk("System:", msg.body(), TalkWindow::OTHER);
		} else {
			window->NewMessage(group_username, msg.body());
		}
		window->Unlock();
	}
}


bool
TalkManager::handleMUCRoomCreation(gloox::MUCRoom *room __attribute__((unused)))
{
	fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
	return false;
}


void
TalkManager::handleMUCSubject(gloox::MUCRoom *room __attribute__((unused)),
							const std::string &nick __attribute__((unused)),
							const std::string &subject __attribute__((unused)))
{
	fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}


void
TalkManager::handleMUCInviteDecline(gloox::MUCRoom *room __attribute__((unused)),
							const gloox::JID &invitee __attribute__((unused)),
							const std::string &reason __attribute__((unused)))
{
	fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}


void
TalkManager::handleMUCError(gloox::MUCRoom *room,
							gloox::StanzaError error)
{
	fprintf(stderr, "%s(%p, %d)\n", __PRETTY_FUNCTION__, room, error);
}


void
TalkManager::handleMUCInfo(gloox::MUCRoom *room __attribute__((unused)), int features __attribute__((unused)),
							const std::string &name __attribute__((unused)),
							const gloox::DataForm *infoForm __attribute__((unused)))
{
	fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}


void
TalkManager::handleMUCItems(gloox::MUCRoom *room __attribute__((unused)),
							const gloox::Disco::ItemList &items __attribute__((unused)))
{
	fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}
