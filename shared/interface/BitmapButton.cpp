//////////////////////////////////////////////////
// BitmapButton [BitmapButton.cpp]
//////////////////////////////////////////////////

#include <cstring>
#include <string>

#include "BitmapButton.h"
#include "translation/TranslationUtils.h"

BitmapButton::BitmapButton(BRect size, const char *name, BMessage *msg, uint32 resizing_mode, uint32 flags)
	: BButton(size, name, NULL, msg, resizing_mode, flags) {
	SetViewColor(B_TRANSPARENT_COLOR);

	_up_state = NULL;
	_down_state = NULL;
	_disabled_state = NULL;
}

BitmapButton::~BitmapButton() {
	delete _up_state;
	delete _down_state;
	delete _disabled_state;
}

void BitmapButton::Draw(BRect update_frame) {
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	if (!IsEnabled()) {
		// button is disabled
		if (_disabled_state) {
			DrawBitmap(_disabled_state, BPoint(0, 0));
		}
	} else if (Value() == B_CONTROL_ON) {
		// button is in the down state
		if (_down_state) {
			DrawBitmap(_down_state, BPoint(0, 0));
		}
	} else if (Value() == B_CONTROL_OFF) {
		// button is in the up state
		if (_up_state) {
			DrawBitmap(_up_state, BPoint(0, 0));
		}
	}
}
		
bool BitmapButton::SetUpState(string filename) {
	// set the new state
	_up_state = BTranslationUtils::GetBitmap(filename.c_str());

	// force redraw
	Invalidate();
	
	// return status	
	return (_up_state != NULL);
}

bool BitmapButton::SetDownState(string filename) {
	// set the new state
	_down_state = BTranslationUtils::GetBitmap(filename.c_str());

	// force redraw
	Invalidate();
	
	// return status	
	return (_down_state != NULL);
}

bool BitmapButton::SetDisabledState(string filename) {
	// set the new state
	_disabled_state = BTranslationUtils::GetBitmap(filename.c_str());

	// force redraw
	Invalidate();
	
	// return status	
	return (_disabled_state != NULL);
}