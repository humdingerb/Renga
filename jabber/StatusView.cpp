//////////////////////////////////////////////////
// Interface [StatusView.cpp]
//////////////////////////////////////////////////

#ifndef STATUS_VIEW_H
	#include "StatusView.h"
#endif

#include <ScrollView.h>

StatusView::StatusView(const char *name)
	: BView(name, B_WILL_DRAW) {
	// set font
	SetFont(be_plain_font);
	SetFontSize(9.0);

	GetFontHeight(&_fh);
	_height = _fh.ascent + _fh.descent + 1.0;
	if (_height <  B_V_SCROLL_BAR_WIDTH)
		_height = B_V_SCROLL_BAR_WIDTH;
	SetExplicitSize(BSize(B_SIZE_UNSET, _height));
	SetExplicitMinSize(BSize(B_SIZE_UNSET, _height));
}

StatusView::~StatusView() {
}

void StatusView::AttachedToWindow() {
}

void StatusView::Draw(__attribute__((unused)) BRect rect) {
	// draw name
	SetHighColor(0, 0, 0, 255);
	DrawString(_current_message.c_str(), BPoint(5, _height - 3));
}

void StatusView::SetMessage(std::string message) {
	_current_message = message;

	// redraw self
	Invalidate();
}

const std::string StatusView::Message() const {
	return _current_message;
}

float StatusView::GetHeight() const {
	return _height;
}
