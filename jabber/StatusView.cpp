//////////////////////////////////////////////////
// Interface [StatusView.cpp]
//////////////////////////////////////////////////

#ifndef STATUS_VIEW_H
	#include "StatusView.h"
#endif

#include <ScrollView.h>
#include <ControlLook.h>

StatusView::StatusView(const char *name)
	: BView(name, B_WILL_DRAW) {
	SetFont(be_plain_font);

	float viewSize = be_control_look->GetScrollBarWidth();
	float fontSize = 9.0f;

	// calculate a font Size that fits into the alocated space
	while (fontSize < 48) {
		SetFontSize(fontSize + 1);
		GetFontHeight(&_fh);
		_height = _fh.ascent + _fh.descent + 1.0f;
		if (_height < viewSize) {
			fontSize = fontSize +1;
			continue;
		}
		if (_height > viewSize) {
			SetFontSize(fontSize);
			break;
		}
		if (_height == viewSize)
			break;
	}

	SetExplicitSize(BSize(B_SIZE_UNSET, viewSize));
	SetExplicitMinSize(BSize(B_SIZE_UNSET, viewSize));
}

StatusView::~StatusView() {
}

void StatusView::AttachedToWindow() {
}

void StatusView::Draw(__attribute__((unused)) BRect rect) {
	// draw name
	SetHighUIColor(B_PANEL_TEXT_COLOR);
	DrawString(_current_message.c_str(), BPoint(5, _height -(_fh.descent + 1)));
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
