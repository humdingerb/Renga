/*
 * Copyright 2019-2021 Adrien Destugues <pulkomandy@pulkomandy.tk>
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
	, fFlags(0)
{
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
	BRect selectionFrame = frame;
	selectionFrame.left = 0;
	if (IsSelected()) {
		owner->SetHighUIColor(B_LIST_SELECTED_BACKGROUND_COLOR);
	} else {
		owner->SetHighColor(owner->ViewColor());
	}
	owner->FillRect(selectionFrame);

	// draw a graphic
	owner->SetDrawingMode(B_OP_ALPHA);
	owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	// TODO this is probably not so great at larger font sizes, but at size 10
	// it gives the proper alignment of the text and icon. Figure out a better
	// way to make sure they are aligned, probably based on the frame height
	// and trying to center the icon in it
	BPoint iconPosition(frame.left - 2, frame.top);
	if (status == UserID::TRANSPORT_ONLINE) {
		owner->DrawBitmapAsync(_online_icon, iconPosition);
	} else if (status == UserID::UNKNOWN) {
		owner->DrawBitmapAsync(_unknown_icon, iconPosition);
	}

	BString name = Text();
	if (name.IsEmpty())
		name = _userid.full().c_str();

	if (IsSelected()) {
		owner->SetHighUIColor(B_LIST_SELECTED_ITEM_TEXT_COLOR);
		// When the room is active, we can clear the flags
		fFlags = 0;
	} else if (fFlags & NICKNAME_HIGHLIGHT)
		owner->SetHighUIColor(B_FAILURE_COLOR);
	else if (fFlags & ACTIVITY)
		owner->SetHighUIColor(B_SUCCESS_COLOR);
	else
		owner->SetHighUIColor(B_LIST_ITEM_TEXT_COLOR);

	// construct text positioning, keeping space for the icon on the left
	font_height fh;
	owner->GetFontHeight(&fh);
	float height = fh.ascent + fh.descent;

	owner->DrawString(name, BPoint(frame.left + height, frame.bottom - fh.descent));
}


void BookmarkItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);

	const gloox::ConferenceListItem* item
		= BookmarkManager::Instance().GetBookmark(_userid.full().c_str());
	if (item != NULL && !item->name.empty())
		SetText(item->name.c_str());

	font_height fh;
	owner->GetFontHeight(&fh);
	float height = floorf(fh.ascent + fh.descent);
	SetHeight(height);

	if (_online_icon == NULL) {
		height = height * 16 / 11;
			// Adjust for the empty space in the icons (they use the same
			// format as the BToolbar icons and don't fill the whole space for
			// some reason)
		printf("size: %f\n", height);
		_online_icon = LoadIconFromResource("online", height);
		_unknown_icon = LoadIconFromResource("unknown", height);
	}
}


const gloox::JID& BookmarkItem::GetUserID() const {
	return _userid;
}
