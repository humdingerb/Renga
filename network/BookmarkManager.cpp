/*
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include "BookmarkManager.h"

#include "jabber/TalkManager.h"

#include <Message.h>


BookmarkManager& BookmarkManager::Instance()
{
	static BookmarkManager instance;
	return instance;
}


void BookmarkManager::Connect()
{
	if (fBookmarks != NULL || fBookmarks2 != NULL) {
		debugger("Already connected");
	}

	gloox::Client* client = JabberSpeak::Instance()->GlooxClient();
	// Request for bookmarks
	// We could always use bookmarks2:
	// - If the server supports migration from the older version, that's great
	// - If it doesn't, we can still store and retrieve bookmarks this way,
	//   they just won't sync up with clients using the old format
	// However, since few servers support them, for now it makes more sense to
	// keep with bookmarks1. Later we can detect servers which expose "bookmark
	// migration" and use bookmarks2 only for those.
	if (true) {
		fBookmarks = std::make_unique<gloox::BookmarkStorage>(client);
		fBookmarks->registerBookmarkHandler(this);
		fBookmarks->requestBookmarks();
	} else {
		fBookmarks2 = std::make_unique<gloox::Bookmark2Storage>(client);
		fBookmarks2->registerBookmarkHandler(this);
		fBookmarks2->requestBookmarks();
	}
}


void BookmarkManager::Disconnect()
{
	fBookmarks = nullptr;
	fBookmarks2 = nullptr;
}


void
BookmarkManager::SetBookmark(BString jid, BString nick, BString name, bool autojoin)
{
	// Do nothing id we are disconnected
	if (!fBookmarks && !fBookmarks2)
		return;

	gloox::ConferenceListItem item;
	bool update = false;

	// Remove previous version of the item
	for (auto i = fConferences.begin(); i != fConferences.end(); i++) {
		if (i->jid == jid.String()) {
			item = *i;
			update = true;
			fConferences.erase(i);
			break;
		}
	}


	item.nick = nick;
	item.jid = jid;
	item.autojoin = autojoin;
	item.name = name;

	if (!update) {
		item.password = gloox::EmptyString;
	}

	fConferences.push_back(item);

	if (fBookmarks2) {
		// Bookmakrs2 is nice and easy, each bookmark is stored independently.
		fBookmarks2->storeBookmark(item);
	} else if (fBookmarks) {
		// We need to re-send the whole list everytime :(
		fBookmarks->storeBookmarks(fURIBookmarks, fConferences);
	}

	// Notify about the updated entry
	BMessage* bookmarks = new BMessage(kBookmarks);
	bookmarks->AddString("jid", jid);
	bookmarks->AddString("nick", nick);
	bookmarks->AddBool("autojoin", autojoin);
	//bookmarks->AddString("name", i.name.c_str());
	//bookmarks->AddString("password", i.password.c_str());
	SendNotices(kBookmarks, bookmarks);
}


const gloox::ConferenceListItem*
BookmarkManager::GetBookmark(BString jid)
{
	// Remove previous version of the item
	for (auto i = fConferences.begin(); i != fConferences.end(); i++) {
		if (i->jid == jid.String()) {
			return &*i;
		}
	}

	return NULL;
}


void
BookmarkManager::RemoveBookmark(BString jid)
{
	// Do nothing id we are disconnected
	if (!fBookmarks && !fBookmarks2)
		return;

	gloox::ConferenceListItem item;
	bool update = false;

	// Remove previous version of the item
	for (auto i = fConferences.begin(); i != fConferences.end(); i++) {
		if (i->jid == jid.String()) {
			item = *i;
			update = true;
			fConferences.erase(i);
			break;
		}
	}

	if (!update) {
		return;
	}


	if (fBookmarks2) {
		// Bookmakrs2 is nice and easy, each bookmark is stored independently.
		fBookmarks2->removeBookmark(item);
	} else if (fBookmarks) {
		// We need to re-send the whole list everytime :(
		fBookmarks->storeBookmarks(fURIBookmarks, fConferences);
	}

	BMessage* bookmarks = new BMessage(kBookmarks);
	bookmarks->AddString("jid", jid);
	bookmarks->AddBool("delete", true);
	SendNotices(kBookmarks, bookmarks);
}


void
BookmarkManager::handleBookmarks(const gloox::BookmarkList& bList,
                             const gloox::ConferenceList& cList)
{
	fURIBookmarks = bList; // unused but preserved
	fConferences = cList;

	if (cList.empty()) {
		puts("Conference list is empty");
		return;
	}

	BMessage* bookmarks = new BMessage(kBookmarks);
	// bookmark list (for URLs) is ignored, no one cares about it
	for (auto i: cList) {
		if (i.autojoin) {
			TalkManager::Instance()->CreateTalkSession(gloox::Message::Groupchat, NULL,
				i.jid.c_str(), i.nick.c_str(), NULL);
		}
		bookmarks->AddString("name", i.name.c_str());
		bookmarks->AddString("jid", i.jid.c_str());
		bookmarks->AddString("nick", i.nick.c_str());
		bookmarks->AddString("password", i.password.c_str());
		bookmarks->AddBool("autojoin", i.autojoin);

	}
	SendNotices(kBookmarks, bookmarks);
}
