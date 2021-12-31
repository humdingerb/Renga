//////////////////////////////////////////////////
// Blabber [RosterItem.cpp]
//////////////////////////////////////////////////

#include "RosterItem.h"

#include "support/AppLocation.h"

#include <TranslationUtils.h>

#include "ui/HVIFUtil.h"

BBitmap *RosterItem::_kinda_online_icon = NULL;
BBitmap *RosterItem::_offline_icon      = NULL;
BBitmap *RosterItem::_online_icon       = NULL;
BBitmap *RosterItem::_unknown_icon      = NULL;


RosterItem::RosterItem(const UserID *userid)
	: BStringItem(userid->FriendlyName().c_str())
	, fAvatar(NULL)
{
	_userid           = userid;
	_is_stale_pointer = false;

	// intitialize static members
	if (_offline_icon == NULL) {
		_kinda_online_icon = LoadIconFromResource("away-online");
		_online_icon = LoadIconFromResource("online");
		_offline_icon = LoadIconFromResource("offline");
		_unknown_icon = LoadIconFromResource("unknown");
	}
}


RosterItem::~RosterItem()
{
	delete fAvatar;
}


void RosterItem::SetAvatar(BBitmap* avatar)
{
	delete fAvatar;
	fAvatar = avatar;
}


void RosterItem::DrawItem(BView *owner, BRect frame, __attribute__((unused)) bool complete) {
	// protection
	if (StalePointer()) {
		return;
	}

	// get online status
	UserID::online_status status = _userid->OnlineStatus();
	std::string                exact_status = _userid->ExactOnlineStatus();

	// text characteristics
	owner->SetFont(be_plain_font);

	// clear rectangle
	if (IsSelected()) {
		owner->SetHighUIColor(B_LIST_SELECTED_BACKGROUND_COLOR);
	} else {
		owner->SetHighColor(owner->ViewColor());
	}

	BRect fullFrame(frame);
	fullFrame.left = 0;
	owner->FillRect(fullFrame);

	// draw a graphic
	owner->SetDrawingMode(B_OP_ALPHA);
	owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	int height = frame.bottom - frame.top;
	frame.left = height - 4; // have the status badge right over the edge of the avatar
	if (fAvatar) {
		owner->DrawBitmapAsync(fAvatar, fAvatar->Bounds(),
			BRect(0, frame.top, height - 1, frame.bottom - 1), B_FILTER_BITMAP_BILINEAR);
	}

	BPoint iconPosition(frame.left - 3, frame.top + 1);
	if (status == UserID::ONLINE) {
		if (exact_status == "xa" || exact_status == "away" || exact_status == "dnd") {
			if (_kinda_online_icon) {
				owner->DrawBitmapAsync(_kinda_online_icon, iconPosition);
			}
		} else {
			if (_online_icon) {
				owner->DrawBitmapAsync(_online_icon, iconPosition);
			}
		}
	} else if (status == UserID::OFFLINE) {
		if (_offline_icon) {
			owner->DrawBitmapAsync(_offline_icon, iconPosition);
		}
	} else {
		if (_unknown_icon) {
			owner->DrawBitmapAsync(_unknown_icon, iconPosition);
		}
	}

	// construct name
	std::string name = GetUserID()->FriendlyName();

	if (name.empty()) {
		name = GetUserID()->Handle();

		if (name.empty()) {
			name = "<anonymous>";
		}
	}

	BFont statusFont;

	// font color is based on online status
	if (status == UserID::ONLINE) {
		if (exact_status == "dnd") {
			owner->SetHighColor(255, 0, 0, 255);
			owner->GetFont(&statusFont);
			statusFont.SetFace(B_ITALIC_FACE);
			owner->SetFont(&statusFont);
		}
		else if(exact_status == "xa" || exact_status == "away")
		{
			owner->SetHighColor(215, 107, 0, 255);
			owner->GetFont(&statusFont);
			statusFont.SetFace(B_ITALIC_FACE);
			owner->SetFont(&statusFont);
		}
		 else {
			owner->SetHighColor(0, 180, 0, 255);
		}
	} else if (status == UserID::OFFLINE) {
		owner->SetHighColor(120, 120, 120, 255); //gray
	} else {
		owner->SetHighColor(0, 0, 255, 255); //blue
	}

	// construct text positioning
	font_height fh;
	owner->GetFontHeight(&fh);

	// draw name
	BPoint textPosition(frame.left + 13, frame.top + fh.ascent + 1);
	owner->DrawString(name.c_str(), textPosition);

	// draw show
	if (!GetUserID()->MoreExactOnlineStatus().empty()) {
		owner->SetHighColor(0, 0, 0, 255);
		textPosition.y += frame.Height() / 2;

		owner->DrawString(GetUserID()->MoreExactOnlineStatus().c_str(), textPosition);
	}
}

void RosterItem::Update(BView *owner, const BFont *font) {
	BListItem::Update(owner, font);

	font_height fh;
	owner->GetFontHeight(&fh);

	float height = fh.ascent + fh.descent;

	// set height to accomodate graphics and text
	SetHeight(height * 2 + 3);
}

bool RosterItem::StalePointer() const {
	return _is_stale_pointer;
}

const UserID *RosterItem::GetUserID() const {
	if (StalePointer()) {
		return NULL;
	} else {
		return _userid;
	}
}

void RosterItem::SetStalePointer(bool is_stale) {
	_is_stale_pointer = is_stale;
}
