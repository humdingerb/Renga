/*
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include "BookmarkItem.h"

#include "../jabber/TalkManager.h"

#include "../network/BookmarkManager.h"

#include <gloox/bookmarkhandler.h>

#include <String.h>
#include <TranslationUtils.h>

#include "ui/HVIFUtil.h"

BBitmap *BookmarkItem::_online_icon = NULL;
BBitmap *BookmarkItem::_unknown_icon = NULL;

BookmarkItem::BookmarkItem(const gloox::JID& userid, BString name)
	: BStringItem(name)
	, _userid(userid)
{
	// intitialize static members
	if (_online_icon == NULL) {
		_online_icon = LoadIconFromResource("online");
		_unknown_icon = LoadIconFromResource("unknown");
	}
}

BookmarkItem::~BookmarkItem() {
}

void BookmarkItem::DrawItem(BView *owner, BRect frame, __attribute__((unused)) bool complete)
{
	// get online status
	UserID::online_status status;
	if (TalkManager::Instance()->IsExistingWindowToGroup(_userid.full()) != nullptr)
		status = UserID::TRANSPORT_ONLINE;
	else
		status = UserID::UNKNOWN;

	// clear rectangle
	if (IsSelected()) {
		if (status == UserID::TRANSPORT_ONLINE) {
			owner->SetHighColor(200, 255, 200, 255);
		} else if (status == UserID::UNKNOWN) {
			owner->SetHighColor(200, 200, 255, 255);
		}
	} else {
		owner->SetHighColor(owner->ViewColor());
	}

	owner->FillRect(frame);

	// draw a graphic
	owner->SetDrawingMode(B_OP_ALPHA);
	owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	BPoint iconPosition(frame.left - 2, frame.top);
	if (status == UserID::TRANSPORT_ONLINE) {
		owner->DrawBitmapAsync(_online_icon, iconPosition);
	} else if (status == UserID::UNKNOWN) {
		owner->DrawBitmapAsync(_unknown_icon, iconPosition);
	}

	float height;

	BString name = Text();
	if (name.IsEmpty())
		name = _userid.full().c_str();

	// font color is based on online status
	if (status == UserID::OFFLINE) {
		owner->SetHighColor(255, 0, 0, 255);
	} else if (status == UserID::UNKNOWN) {
		owner->SetHighColor(0, 0, 255, 255);
	} else if (status == UserID::TRANSPORT_ONLINE) {
		owner->SetHighColor(0, 180, 0, 255);
	} else {
		owner->SetHighColor(0, 0, 0, 255);
	}

	// construct text positioning
	font_height fh;
	owner->GetFontHeight(&fh);

	height = fh.ascent + fh.descent;

	owner->DrawString(name, BPoint(frame.left + 13, frame.bottom - ((frame.Height() - height) / 2) - fh.descent));
}

void BookmarkItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);

	const gloox::ConferenceListItem* item
		= BookmarkManager::Instance().GetBookmark(_userid.full().c_str());
	if (item != NULL && !item->name.empty())
		SetText(item->name.c_str());

	// set height to accomodate graphics and text
	SetHeight(16.0);
}

const gloox::JID& BookmarkItem::GetUserID() const {
	return _userid;
}
