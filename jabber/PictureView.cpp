#include "PictureView.h"
#include <TranslationKit.h>

PictureView::PictureView(const char *filename, uint32 flags)
	: BView(NULL, flags) {
	_bitmap = NULL;
	float width  = 0.0f;
	float height = 0.0f;

	// transparency
	SetViewColor(B_TRANSPARENT_COLOR);

	// try to get the image
	_bitmap = BTranslationUtils::GetBitmap('PiNG', filename);

	if (_bitmap) {
		BRect frame(_bitmap->Bounds());

		width  = frame.Width();
		height = frame.Height();
	} else {
		return;
	}
	
	// finally, now that we know our filesize
	SetExplicitSize(BSize(width, height));
}

PictureView::~PictureView() {
	delete _bitmap;
}

void PictureView::AttachedToWindow() {
}

void PictureView::Draw(__attribute__((unused)) BRect frame) {
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	if (_bitmap) {
		DrawBitmap(_bitmap, BPoint(0, 0));
	}
}

bool PictureView::InitCheck() {
	return (_bitmap != NULL);
}
