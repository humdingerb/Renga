//////////////////////////////////////////////////
// Blabber [TalkManager.cpp]
//////////////////////////////////////////////////

#include <gloox/carbons.h>
#include <gloox/instantmucroom.h>
#include <gloox/rostermanager.h>

#include <cstdio>
#include <stdexcept>

#include <interface/Window.h>

#include "ui/ModalAlertFactory.h"

#include "BlabberSettings.h"
#include "JabberSpeak.h"
#include "MessageRepeater.h"
#include "Messages.h"
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
		if (session && fTalkMap.find(session) != fTalkMap.end()) {
			window = fTalkMap.at(session);
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
		
			// add it to the known list
			fTalkMap[session] = window;
			SendNotices(kWindowList);
			session->registerMessageHandler(this);
		}
	} else {
		if (IsExistingWindowToGroup(group_room))
			window = fGroupMap.at((gloox::MUCRoom*)IsExistingWindowToGroup(group_room));
		else {
			// create a new window
			window = new TalkView(user, group_room, group_username, NULL);

			gloox::JID jid(group_room);
			jid.setResource(group_username);
			gloox::MUCRoom* room = new gloox::InstantMUCRoom(
					JabberSpeak::Instance()->GlooxClient(), jid, this);

			// add it to the known list
			fGroupMap[room] = window;

			room->join();
			SendNotices(kWindowList);
		}
	}

	BMessage message(kAddTalkView);
	message.AddPointer("view", window);
	BlabberMainWindow::Instance()->PostMessage(&message);

	// return a reference as well
	return window;
}


void TalkManager::handleMessage(const gloox::Message& msg, gloox::MessageSession* session)
{
	// First check if it's a carbon
	if (msg.hasEmbeddedStanza()) {
		// get the possible carbon extension
		const gloox::Carbons *carbon = msg.findExtension<const gloox::Carbons>(
			gloox::ExtCarbons);

		// if the extension exists and contains a message, use it as the real message
		if (carbon && carbon->embeddedStanza()) {
			const gloox::Message* message = static_cast<gloox::Message*>(
				carbon->embeddedStanza());

			try {
				TalkView* window = fTalkMap.at(session);
				window->LockLooper();
				window->AddToTalk(window->OurRepresentation().c_str(), message->body(), TalkView::LOCAL);
				window->UnlockLooper();
			} catch (const std::out_of_range&) {
				// In case we get a carbon for a chat we have not joined?
			}

			return;
		}
	}

	TalkView* window = fTalkMap.at(session);

	// submit the chat FIXME use a BMessage and let the view handle it asynchronously
	window->LockLooper();
	window->NewMessage(msg.body());
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

	// create the window
	CreateTalkSession((gloox::Message::MessageType)session->types(),
		&session->target(), "", "", session, true);
}


void* TalkManager::IsExistingWindowToGroup(string group_room) {
	// check names
	for (auto i = fGroupMap.begin(); i != fGroupMap.end(); ++i) {
		if ((*i).second->GetGroupRoom() == group_room) {
			return (*i).first;
		}
	}

	// no matches
	return nullptr;
}


void TalkManager::RemoveWindow(TalkView* window) {
	for (GroupMap::iterator i = fGroupMap.begin(); i != fGroupMap.end(); ++i) {
		if ((*i).second == window) {
			delete (*i).first;
			fGroupMap.erase(i);
			SendNotices(kWindowList);
			return;
		}
	}

	for (TalkMap::iterator i = fTalkMap.begin(); i != fTalkMap.end(); ++i) {
		if ((*i).second == window) {
			fTalkMap.erase(i);
			SendNotices(kWindowList);
			return;
		}
	}
}


void TalkManager::Reset() {
	MessageRepeater::Instance()->PostMessage(JAB_CLOSE_TALKS);
	fTalkMap.clear();
	fGroupMap.clear();
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
	msg.AddInt32("affiliation", participant.affiliation);
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
	TalkView *window = NULL;

	string group_username;

	// clear out text
	group_username = msg.from().resource();

	window = fGroupMap.at(room);
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
TalkManager::handleMUCSubject(gloox::MUCRoom *room,
							const std::string &nick,
							const std::string &subject)
{
	TalkView* window = fGroupMap.at(room);
	BString topic;
	topic.SetToFormat("set topic to %s\n", subject.c_str());

	// FIXME just send a BMessage to the view and let it handle this
	window->LockLooper();
	window->SetStatus(subject);
	if (!nick.empty()) {
		// Do it only for topic chnages (not for the initial topic setting)
		window->AddToTalk(nick, topic.String(), TalkView::OTHER);
	}
	window->UnlockLooper();
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
