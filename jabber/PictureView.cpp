#include "PictureView.h"
#include <TranslationKit.h>

PictureView::PictureView(const char *filename, BPoint point, uint32 resizing_flags, uint32 flags)
	: BView(BRect(0, 0, 0, 0), NULL, resizing_flags, flags) {
	_bitmap = NULL;
	_width  = 0.0;
	_height = 0.0;

	// transparency
	SetViewColor(B_TRANSPARENT_COLOR);

	// try to get the image
	_bitmap = BTranslationUtils::GetBitmap(filename);

	if (_bitmap) {
		BRect frame(_bitmap->Bounds());

		_width  = frame.Width();
		_height = frame.Height();
	} else {
		return;
	}
	
	// finally, now that we know our filesize
	ResizeTo(_width, _height);
	MoveTo(point);
}

PictureView::~PictureView() {
	delete _bitmap;
}

void PictureView::AttachedToWindow() {
}

void PictureView::Draw(BRect frame) {
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	if (_bitmap) {
		DrawBitmap(_bitmap, BPoint(0, 0));
	}
}

bool PictureView::InitCheck() {
	return (_bitmap != NULL);
}
