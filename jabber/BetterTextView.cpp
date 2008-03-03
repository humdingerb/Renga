//////////////////////////////////////////////////
// Blabber [BetterTextView.cpp]
//////////////////////////////////////////////////

#ifndef BETTER_TEXT_VIEW_H
	#include "BetterTextView.h"
#endif

BetterTextView::BetterTextView(BRect frame, const char *name, BRect text_rect, uint32 resizing_mode, uint32 flags)
	: BTextView(frame, name, text_rect, resizing_mode, flags) {
}

BetterTextView::BetterTextView(BRect frame, const char *name, BRect text_rect, const BFont *font, const rgb_color *color, uint32 resizing_mode, uint32 flags)
	: BTextView(frame, name, text_rect, font, color, resizing_mode, flags) {
}

void BetterTextView::FrameResized(float width, float height) {
	BTextView::FrameResized(width, height);

	BRect rect(Frame());

	rect.OffsetTo(B_ORIGIN);
	
	rect.InsetBy(2.0, 2.0);
	
	SetTextRect(rect);

	Invalidate();
}

void BetterTextView::KeyDown(const char *bytes, int32 numbytes) {
	// base class version
	BTextView::KeyDown(bytes, numbytes);
};