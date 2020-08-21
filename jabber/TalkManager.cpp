//////////////////////////////////////////////////
// Blabber [TalkManager.cpp]
//////////////////////////////////////////////////

#include <gloox/carbons.h>
#include <gloox/instantmucroom.h>
#include <gloox/rostermanager.h>

#include <cstdio>
#include <stdexcept>

#include <interface/Window.h>

#include "BlabberSettings.h"
#include "JabberSpeak.h"
#include "MessageRepeater.h"
#include "Messages.h"
#include "../ui/ModalAlertFactory.h"
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
	_instance = NULL;
}

TalkView *TalkManager::CreateTalkSession(const gloox::Message::MessageType type,
	const gloox::JID* user, string group_room, string group_username,
	gloox::MessageSession* session, bool sound_on_new)
{
	TalkView *window = NULL;

	// is there a window already?
	if (type != gloox::Message::Groupchat) {
		if (IsExistingWindowToUser(user->bare()).size()) {
			window = _talk_map.at(IsExistingWindowToUser(user->bare()));
		} else {
			if (session == NULL) {
				gloox::JID fullJID = *user;
				gloox::RosterManager* rm = JabberSpeak::Instance()->GlooxClient()->rosterManager();
				gloox::RosterItem* ri = rm->getRosterItem(fullJID);
				if (ri && !ri->resources().empty()) {
					std::string res = ri->resources().begin()->first;
					fullJID.setResource(res);
				}
				session = new gloox::MessageSession(
					JabberSpeak::Instance()->GlooxClient(), fullJID);
			}
			// create a new window
			window = new TalkView(user, group_room, group_username, session);

			if (sound_on_new) {
				// play a sound
				SoundSystem::Instance()->PlayNewMessageSound();
			}
		
			// add it to the known list BUGBUG we need to remove this when window closes
			_talk_map[session->threadID()] = window;
			SendNotices(kWindowList);
		}
	} else {
		if (IsExistingWindowToGroup(group_room).size()) {
			window = _talk_map.at(IsExistingWindowToGroup(group_room));
		} else {
			if (session == NULL) {
				session = new gloox::MessageSession(
					JabberSpeak::Instance()->GlooxClient(), group_room);
			}
			// create a new window
			window = new TalkView(user, group_room, group_username, session);

			// add it to the known list BUGBUG we need to remove this when window closes
			_talk_map[session->threadID()] = window;

			// FIXME we need to free this when leaving the room!
			gloox::JID jid(group_room);
			jid.setResource(group_username);
			(new gloox::InstantMUCRoom(JabberSpeak::Instance()->GlooxClient(),
				jid, this))->join();
			SendNotices(kWindowList);
		}
	}

	session->registerMessageHandler(this);
	BlabberMainWindow::Instance()->AddTalkView(window);

	// return a reference as well
	return window;
}


void TalkManager::handleMessage(const gloox::Message& msg, gloox::MessageSession* session)
{
	// TODO First check if it's a carbon, in which case the session is incorrect :|
	if (msg.hasEmbeddedStanza()) {
		// get the possible carbon extension
		const gloox::Carbons *carbon = msg.findExtension<const gloox::Carbons>(
			gloox::ExtCarbons);

		// if the extension exists and contains a message, use it as the real message
		if (carbon && carbon->embeddedStanza()) {
			const gloox::Message* message = static_cast<gloox::Message*>(
				carbon->embeddedStanza());

			try {
				TalkView* window = _talk_map.at(IsExistingWindowToUser(message->from().full()));
				window->LockLooper();
				window->AddToTalk(window->OurRepresentation().c_str(), message->body(), TalkView::LOCAL);
				window->UnlockLooper();
			} catch (const std::out_of_range&) {
			}

			return;
		}
	}

	TalkView* window;
	try {
		window = _talk_map.at(IsExistingWindowToUser(session->target().full()));
	} catch (const std::out_of_range&) {
		return;
	}

	// submit the chat
	window->LockLooper();

	if (msg.subtype() == gloox::Message::Groupchat) {
		if (msg.from().full().empty()) {
			window->AddToTalk("System:", msg.body(), TalkView::OTHER);
		} else {
			window->NewMessage(msg.from().resource(), msg.body());
		}
	} else {
		window->NewMessage(msg.body());
	}
	window->UnlockLooper();
}


void
TalkManager::handleMessageSession(gloox::MessageSession* session)
{
	// don't create a window for the carbons session, but still register to
	// get the messages.
	if (session->threadID().empty()) {
		session->registerMessageHandler(this);
		return;
	}

	printf("create window for %s\n", session->threadID().c_str());
	
	// create the window
	CreateTalkSession((gloox::Message::MessageType)session->types(),
		&session->target(), "", "", session, true);
}


string
TalkManager::IsExistingWindowToUser(string username)
{
	// check handles (with resource)
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		const gloox::JID& id = i->second->GetUserID();
		if (id == gloox::JID(username)) {
			return (*i).first;
		}
	}

	// check handles (without resource)
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		const gloox::JID& id = i->second->GetUserID();
		if (id.bare() == gloox::JID(username).bare()) {
			return (*i).first;
		}
	}

	// no matches
	return "";
}

string TalkManager::IsExistingWindowToGroup(string group_room) {
	// check names
	for (TalkIter i = _talk_map.begin(); i != _talk_map.end(); ++i) {
		if ((*i).second->GetGroupRoom() == group_room) {
			return (*i).first;
		}
	}

	// no matches
	return "";
}


void TalkManager::RemoveWindow(string thread_id) {
	if (_talk_map.count(thread_id) > 0) {
		_talk_map.erase(thread_id);
		SendNotices(kWindowList);
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
	SendNotices(msg.what, &msg);
}


void
TalkManager::handleMUCMessage(gloox::MUCRoom *room,
	const gloox::Message &msg, bool priv __attribute__((unused)))
{
	string thread_id;
	TalkView *window = NULL;

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
	if (!IsExistingWindowToGroup(group_identity).empty()) {
		thread_id = IsExistingWindowToGroup(group_identity);
	} else {
		fprintf(stderr, "Failed to find chat window\n");
		return;
	}
	window = _talk_map.at(thread_id);
	// submit the chat
	if (window) {
		window->LockLooper();

		if (group_username.empty()) {
			window->AddToTalk("System:", msg.body(), TalkView::OTHER);
		} else {
			window->NewMessage(group_username, msg.body());
		}
		window->UnlockLooper();
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
	fprintf(stderr, "%s(%s, %d)\n", __PRETTY_FUNCTION__, room->name().c_str(), error);
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
