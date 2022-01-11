//////////////////////////////////////////////////
// Blabber [TalkManager.cpp]
//////////////////////////////////////////////////

#include <gloox/base64.h>
#include <gloox/carbons.h>
#include <gloox/instantmucroom.h>
#include <gloox/pubsubevent.h>
#include <gloox/pubsubitem.h>
#include <gloox/rostermanager.h>

#include <cstdio>
#include <stdexcept>

#include <Catalog.h>
#include <File.h>
#include <FindDirectory.h>
#include <Notification.h>
#include <Path.h>
#include <interface/Window.h>

#include "ui/ModalAlertFactory.h"

#include "BlabberSettings.h"
#include "JabberSpeak.h"
#include "MessageRepeater.h"
#include "Messages.h"
#include "SoundSystem.h"
#include "TalkManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TalkManager"


TalkManager *TalkManager::_instance = NULL;

TalkManager *TalkManager::Instance() {
	if (_instance == NULL) {
		_instance = new TalkManager();
	}

	return _instance;
}

TalkManager::TalkManager()
{
	// Create/open the avatars cache directory
	BPath path;
	find_directory(B_USER_CACHE_DIRECTORY, &path);
	BDirectory cacheRoot(path.Path());
	if (!cacheRoot.Contains("Renga")) {
		BDirectory temp;
		cacheRoot.CreateDirectory("Renga", &temp);
		if (!temp.Contains("avatars")) {
			temp.CreateDirectory("avatars", &fAvatarCache);
		}
	}

	fAvatarCache.SetTo(&cacheRoot, "Renga/avatars");
}

TalkManager::~TalkManager() {
	_instance = NULL;
}

void
TalkManager::CreateTalkSession(const gloox::Message::MessageType type,
	const gloox::JID* user, string group_room, string group_username,
	gloox::MessageSession* session, bool sound_on_new)
{
	TalkView *window = NULL;

	// is there a window already?
	if (type != gloox::Message::Groupchat) {
		// This code for when we get called from clicking a user in the roster.
		// There is possibly already a session but the roster doesn't know about it
		// so we have to search it (in a quite inefficient way).
		// FIXME remove all this junk and have each TalkWindow be a SessionHandler
		// for its own session instead of centralizing everything here.
		gloox::JID fullJID = *user;
		if (session == NULL) {
			gloox::RosterManager* rm
				= JabberSpeak::Instance()->GlooxClient()->rosterManager();
			gloox::RosterItem* ri = rm->getRosterItem(fullJID);
			if (ri && !ri->resources().empty()) {
				std::string res = ri->resources().begin()->first;
				fullJID.setResource(res);
			}

			for (TalkMap::iterator i = fTalkMap.begin();
					i != fTalkMap.end(); ++i) {
				if ((*i).first->target().bare() == user->bare()) {
					session = (*i).first;
					break;
				}
			}
		}

		if (session && fTalkMap.find(session) != fTalkMap.end()) {
			window = fTalkMap.at(session);
		} else {
			// Actually create the session, there isn't one matching.
			if (session == NULL) {
				session = new gloox::MessageSession(
						JabberSpeak::Instance()->GlooxClient(), fullJID);
			} else {
				// It's a freshly created session for an incoming message,
				// we just need to create the matching window
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
				BMessage notification(kIncomingMessage);
				notification.AddString("content", message->body().c_str());
				BMessenger(window).SendMessage(&notification);
			} catch (const std::out_of_range&) {
				// In case we get a carbon for a chat we have not joined?
			}

			return;
		}
	}

	const gloox::PubSub::Event* event
		= msg.findExtension<gloox::PubSub::Event>(gloox::ExtPubSubEvent);
	if (event) {
		if (event->node() == "urn:xmpp:avatar:metadata") {
			for (auto item: event->items()) {
				GetAvatar(msg.from(), item->item);
				return;
			}
		} else {
			printf("Got unexpected pubsub event node %s\n", event->node().c_str());
		}
	}

	try {
		TalkView* window = fTalkMap.at(session);
		// submit the chat
		BMessage notification(kIncomingMessage);
		notification.AddString("content", msg.body().c_str());
		BMessenger(window).SendMessage(&notification);
		printf("notify %p\n", window);
	} catch(const std::out_of_range&) {
		printf("%s: no window found for session %p with %s, ignoring message\n",
			__func__, session, session ? session->target().full().c_str(): "NULL");
	}
}


void
TalkManager::handleMessageSession(gloox::MessageSession* session)
{
	// don't create a window for the carbons session, but still register to
	// get the messages.
	if (session->threadID().empty() && session->types() == gloox::Message::Headline) {
		session->registerMessageHandler(this);
		return;
	}

	// create the window
	CreateTalkSession((gloox::Message::MessageType)session->types(),
		&session->target(), "", "", session, true);
}


//#pragma mark - PubSub ResultHandler
void TalkManager::handleItem(const gloox::JID&, const std::string&, const gloox::Tag*)
{
	puts(__func__);
}


void TalkManager::handleItems(const std::string&, const gloox::JID& jid,
	const std::string& node, const gloox::PubSub::ItemList& itemList,
	const gloox::Error*)
{
	if (node == "urn:xmpp:avatar:data") {
		for (auto item: itemList) {
			// item->id() has the checksum
			std::string base64 = item->tag()->findChild("data")->cdata();

			// need to remove all newlines from the base64...
			size_t pos;
			for (;;) {
				pos = base64.find('\n');
				if (pos == std::string::npos)
					break;
				base64.erase(pos, 1);
			}

			const std::string decoded = gloox::Base64::decode64(base64);
			BMessage message(kAvatarUpdate);
			message.AddString("jid", jid.full().c_str());
			message.AddData("avatar", B_RAW_TYPE, decoded.data(), decoded.length());
			SendNotices(kAvatarUpdate, &message);

			// store it in cache
			BFile file;
			fAvatarCache.CreateFile(item->id().c_str(), &file);
			file.Write(decoded.data(), decoded.length());
		}
	} else {
		printf("unknown pubsub item in %s(%s, %s)\n", __func__,
			jid.full().c_str(), node.c_str());
	}
}


void TalkManager::handleItemPublication(const std::string&, const gloox::JID&,
	const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleItemDeletion(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleSubscriptionResult(const std::string&, const gloox::JID&, const std::string&, const std::string&, const gloox::JID&, gloox::PubSub::SubscriptionType, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleUnsubscriptionResult(const std::string&, const gloox::JID&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleSubscriptionOptions(const std::string&, const gloox::JID&, const gloox::JID&, const std::string&, const gloox::DataForm*, const std::string&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleSubscriptionOptionsResult(const std::string&, const gloox::JID&, const gloox::JID&, const std::string&, const std::string&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleSubscribers(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::SubscriptionList&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleSubscribersResult(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::SubscriberList*, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleAffiliates(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::AffiliateList*, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleAffiliatesResult(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::AffiliateList*, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleNodeConfig(const std::string&, const gloox::JID&, const std::string&, const gloox::DataForm*, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleNodeConfigResult(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleNodeCreation(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleNodeDeletion(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleNodePurge(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleSubscriptions(const std::string&, const gloox::JID&, const gloox::PubSub::SubscriptionMap&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleAffiliations(const std::string&, const gloox::JID&, const gloox::PubSub::AffiliationMap&, const gloox::Error*)
{
	puts(__func__);
}


void TalkManager::handleDefaultNodeConfig(const std::string&, const gloox::JID&, const gloox::DataForm*, const gloox::Error*)
{
	puts(__func__);
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


void
TalkManager::RemoveWindow(TalkView* window)
{
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


void
TalkManager::Reset()
{
	MessageRepeater::Instance()->PostMessage(JAB_CLOSE_TALKS);
	fTalkMap.clear();
	fGroupMap.clear();
}


void
TalkManager::GetAvatar(const gloox::JID& jid, const std::string& hash)
{
	// check the cache first in case we already have it
	BFile file(&fAvatarCache, hash.c_str(), B_READ_ONLY);
	if (file.InitCheck() == B_OK) {
		off_t size;
		file.GetSize(&size);
		char buffer[size];
		file.Read(buffer, size);

		BMessage message(kAvatarUpdate);
		message.AddString("jid", jid.full().c_str());
		message.AddData("avatar", B_RAW_TYPE, buffer, size);
		SendNotices(kAvatarUpdate, &message);

	} else {
		// Get it from PubSub
		JabberSpeak::Instance()->RequestPubSubItem(jid, "urn:xmpp:avatar:data", hash, this);
	}
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

	string group_username = msg.from().resource();

	window = fGroupMap.at(room);
	// submit the chat
	if (window) {
		window->LockLooper();

		if (group_username.empty()) {
			window->AddToTalk(B_TRANSLATE("System:"), msg.body(), TalkView::OTHER);
		} else {
			bool highlight;
			// Highlight messages when they mention the nickname
			// TODO also use metadata in the message that may indicate an highlight
			if (BString(msg.body().c_str()).IFindFirst(room->nick().c_str()) != B_ERROR) {
				// NOTE: This will erronously pop up for backlog messages aswell, can be improved
				// once we have MAM
				BNotification notification(B_INFORMATION_NOTIFICATION);
				notification.SetGroup(room->name().c_str());
				notification.SetTitle(group_username.c_str());
				notification.SetContent(msg.body().c_str());
				notification.Send();

				BlabberMainWindow::Instance()->FlagBookmarkItem(msg.from().bare(),
					BookmarkItem::NICKNAME_HIGHLIGHT);
				highlight = true;
			} else {
				BlabberMainWindow::Instance()->FlagBookmarkItem(msg.from().bare(),
					BookmarkItem::ACTIVITY);
				highlight = false;
			}

			// TODO: compare with JID instead?
			if (group_username == window->GetGroupUsername())
				window->AddToTalk(group_username, msg.body(), TalkView::LOCAL, highlight);
			else
				window->AddToTalk(group_username, msg.body(), TalkView::MAIN_RECIPIENT, highlight);
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
	topic.SetToFormat(B_TRANSLATE("Set topic to %s\n"), subject.c_str());

	// FIXME just send a BMessage to the view and let it handle this
	window->LockLooper();
	window->SetStatus(subject);
	window->AddToTalk(nick, topic.String(), TalkView::OTHER);
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
