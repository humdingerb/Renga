//////////////////////////////////////////////////
// Jabber [PeopleListItem.cpp]
//////////////////////////////////////////////////

#ifndef PEOPLE_LIST_ITEM_H
	#include "PeopleListItem.h"
#endif

#ifndef _FONT_H
	#include <interface/Font.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

#ifndef JABBER_SPEAK_H
	#include "JabberSpeak.h"
#endif

PeopleListItem::PeopleListItem(std::string user, gloox::MUCRoomAffiliation affiliation)
	: BListItem() {
	_user   = user;
	fAffiliation = affiliation;
}

PeopleListItem::~PeopleListItem() {
}

void PeopleListItem::DrawItem(BView *owner, BRect frame, __attribute__((unused)) bool complete) {
	// clear rectangle
	if (IsSelected()) {
		owner->SetLowUIColor(B_LIST_SELECTED_BACKGROUND_COLOR);
		owner->SetHighUIColor(B_LIST_SELECTED_ITEM_TEXT_COLOR);
	} else {
		owner->SetLowUIColor(B_LIST_BACKGROUND_COLOR);
		owner->SetHighUIColor(B_LIST_ITEM_TEXT_COLOR);
	}

	owner->FillRect(frame, B_SOLID_LOW);

	// construct text positioning
	font_height fh;
	owner->GetFontHeight(&fh);

	float height = fh.ascent + fh.descent;
	float vpos = frame.bottom - (frame.Height() - height) / 2 - fh.descent;

	// draw information
	owner->DrawString(User().c_str(), BPoint(frame.left + 5.0, vpos));

	BPoint affiliationPos(frame.right - owner->StringWidth("@") - 5.0, vpos);
	if (fAffiliation == gloox::AffiliationOwner)
		owner->DrawString("&", affiliationPos);
	else if (fAffiliation == gloox::AffiliationAdmin)
		owner->DrawString("@", affiliationPos);
}

void PeopleListItem::Update(BView *owner, const BFont *font) {
	BListItem::Update(owner, font);

	// set height to accomodate graphics and text
	SetHeight(13.0);
}

std::string PeopleListItem::User() const {
	return _user;
}
