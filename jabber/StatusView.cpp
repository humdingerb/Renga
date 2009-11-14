//////////////////////////////////////////////////
// Interface [StatusView.cpp]
//////////////////////////////////////////////////

#ifndef STATUS_VIEW_H
	#include "StatusView.h"
#endif

StatusView::StatusView(const char *name)
	: BView(BRect(0, 0, 0, 0), name, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW) {
	// set font
	SetFont(be_plain_font);
	SetFontSize(9.0);

	// construct text positioning
	GetFontHeight(&_fh);

	_height = _fh.ascent + _fh.descent;
}

StatusView::~StatusView() {
}

void StatusView::AttachedToWindow() {
	// resize to connect to bottom of parent
	BRect rect = Parent()->Bounds();

	// set new size
	ResizeTo(rect.Width(), _height + 3.0);
	MoveTo(0.0, rect.bottom - _height - 2.0);
}

void StatusView::Draw(BRect rect) {
	// draw indent
	SetHighColor(152, 152, 152, 255);
	StrokeLine(BPoint(0.0, 0.0), BPoint(Bounds().Width(), 0.0));

	SetHighColor(255, 255, 255, 255);
	StrokeLine(BPoint(0.0, 1.0), BPoint(Bounds().Width(), 1.0));

	// draw name
	SetHighColor(0, 0, 0, 255);
	DrawString(_current_message.c_str(), BPoint(5, Bounds().bottom - ((Bounds().Height() - _height - 2) / 2) - _fh.descent));
}

void StatusView::SetMessage(std::string message) {
	_current_message = message;

	// redraw self
	Invalidate();
}

const std::string StatusView::Message() const {
	return _current_message;
}

const float StatusView::GetHeight() const {
	return _height;
}
