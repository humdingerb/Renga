/*
 * BookmarkkItem.h
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

/** @file
 * Bookmark (group chat) entries of the RosterView widget
 */

#pragma once

#include <Bitmap.h>
#include <ListItem.h>
#include <String.h>
#include <View.h>

#include "../jabber/UserID.h"

class BookmarkItem : public BStringItem {
public:
			         BookmarkItem(const gloox::JID& userid, BString name);
  			        ~BookmarkItem();

	void             DrawItem(BView *owner, BRect frame, bool complete = false);
	virtual void     Update(BView *owner, const BFont *font);
	
	const gloox::JID& GetUserID() const;
	
private:
	const gloox::JID _userid;
	BString fName;
		
	static BBitmap *_online_icon;
	static BBitmap *_unknown_icon;
};
