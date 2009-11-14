//////////////////////////////////////////////////
// Shared Interface [BubbleView.cpp]
//////////////////////////////////////////////////

#ifndef BUBBLE_VIEW_H
	#include "BubbleView.h"
#endif

BubbleView::BubbleView(BRect frame, const char *name, const char *text, rgb_color bg, rgb_color fg)
	: BStringView(frame, name, text) {
	_bg = bg;
	_fg = fg;
	
	SetFont(be_bold_font);
	SetFontSize(15.0);

	SetViewColor(_bg);
	SetLowColor(_bg);
	SetHighColor(_fg);
}

BubbleView::~BubbleView() {
}

void BubbleView::Draw(BRect update) {
	SetHighColor(_bg);
	FillRect(Bounds());
	
	SetHighColor(_fg);
	StrokeRect(Bounds());

	SetLowColor(_bg);
	SetViewColor(_bg);
	
	BStringView::Draw(update);
}