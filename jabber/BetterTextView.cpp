//////////////////////////////////////////////////
// Blabber [BetterTextView.cpp]
//////////////////////////////////////////////////

#ifndef BETTER_TEXT_VIEW_H
	#include "BetterTextView.h"
#endif

BetterTextView::BetterTextView(const char *name, uint32 flags)
	: BTextView(name, flags) {
}

BetterTextView::BetterTextView(const char *name, const BFont *font, const rgb_color *color, uint32 flags)
	: BTextView(name, font, color, flags) {
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
