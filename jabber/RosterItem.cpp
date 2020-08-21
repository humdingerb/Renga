//////////////////////////////////////////////////
// Blabber [RosterItem.cpp]
//////////////////////////////////////////////////

#include "RosterItem.h"

#include "support/AppLocation.h"

#include <TranslationUtils.h>

BBitmap *RosterItem::_kinda_online_icon = NULL;
BBitmap *RosterItem::_offline_icon      = NULL;
BBitmap *RosterItem::_online_icon       = NULL;
BBitmap *RosterItem::_unknown_icon      = NULL;
BBitmap *RosterItem::_icq_icon          = NULL;

RosterItem::RosterItem(const UserID *userid)
	: BStringItem(userid->FriendlyName().c_str()) {
	_userid           = userid;
	_is_stale_pointer = false;

	// intitialize static members
	if (_offline_icon == NULL) {
		_kinda_online_icon = BTranslationUtils::GetBitmap('PiNG', "away-online");
		_online_icon = BTranslationUtils::GetBitmap('PiNG', "online");
		_offline_icon = BTranslationUtils::GetBitmap('PiNG', "offline");
		_unknown_icon = BTranslationUtils::GetBitmap('PiNG', "unknown");
		_icq_icon = BTranslationUtils::GetBitmap('PiNG', "icq");
	}
}

RosterItem::~RosterItem() {
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
	owner->SetFontSize(10.0);

	// clear rectangle
	if (IsSelected()) {
		if (status == UserID::ONLINE) {
			if(exact_status == "xa" || exact_status == "away")
			{
				owner->SetHighColor(255, 179, 0, 255);
			}
			else if(exact_status == "dnd") 
			{
				owner->SetHighColor(213, 158, 158, 255);
			}
			else {
				owner->SetHighColor(158, 213, 158, 255);
			}
		} else if (status == UserID::OFFLINE) {
			owner->SetHighColor(213, 213, 213, 255);
		} else {
			owner->SetHighColor(200, 200, 255, 255);
		}
	} else {
		owner->SetHighColor(owner->ViewColor());
	}

	owner->FillRect(frame);

	// draw a graphic
	owner->SetDrawingMode(B_OP_ALPHA);
	owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	if (status == UserID::ONLINE) {
		if (exact_status == "xa" || exact_status == "away" || exact_status == "dnd") {
			if (_kinda_online_icon) {
				owner->DrawBitmapAsync(_kinda_online_icon, BPoint(frame.left + 1, frame.top + 4));
			}
		} else {
			if (_online_icon) {
				owner->DrawBitmapAsync(_online_icon, BPoint(frame.left + 1, frame.top + 4));
			}
		}
	} else if (status == UserID::OFFLINE) {
		if (_offline_icon) {
			owner->DrawBitmapAsync(_offline_icon, BPoint(frame.left + 1, frame.top + 4));
		}
	} else {
		if (_unknown_icon) {
			owner->DrawBitmapAsync(_unknown_icon, BPoint(frame.left + 1, frame.top + 4));
		}
	}

	float height;

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

	height = fh.ascent + fh.descent;

	// draw name
	owner->DrawString(name.c_str(), BPoint(frame.left + 13, frame.bottom - ((frame.Height() - height) / 2) - fh.descent));

	// draw show
	if (!GetUserID()->MoreExactOnlineStatus().empty()) {
		owner->SetHighColor(0, 0, 0, 255);

		owner->DrawString(" [");
		owner->DrawString(GetUserID()->MoreExactOnlineStatus().c_str());
		owner->DrawString("]");
	}

	// draw external chat icon
	if (GetUserID()->UserType() == UserID::ICQ) {
		if (_icq_icon) {
			owner->DrawBitmapAsync(_icq_icon, BPoint(owner->PenLocation().x + 2.0, frame.top + 2));
		}
	}
	
	owner->SetFont(be_plain_font);
	owner->SetFontSize(10.0);

}

void RosterItem::Update(BView *owner, const BFont *font) {
	BListItem::Update(owner, font);

	// set height to accomodate graphics and text
	SetHeight(16.0);
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
