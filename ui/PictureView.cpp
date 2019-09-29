/*
 * Copyright 19??-2001, John Blanco
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include "PictureView.h"

#include <TranslationKit.h>

#include <stdio.h>

PictureView::PictureView(const char *filename, uint32 flags)
	: BView(NULL, flags | B_WILL_DRAW)
{
	// try to get the image
	_bitmap = BTranslationUtils::GetBitmap('PiNG', filename);

	_Init();
}


PictureView::PictureView(BPositionIO *source, uint32 flags)
	: BView("__picture", flags | B_WILL_DRAW)
{
	// try to get the image
	_bitmap = BTranslationUtils::GetBitmap(source);
	if (_bitmap == NULL)
		puts("FAIL GET BITMAP");

	_Init();
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


void PictureView::_Init()
{
	float width  = 0.0f;
	float height = 0.0f;

	// transparency
	SetViewColor(B_TRANSPARENT_COLOR);

	if (_bitmap) {
		BRect frame(_bitmap->Bounds());

		width  = frame.Width();
		height = frame.Height();

		// finally, now that we know our filesize
		SetExplicitSize(BSize(width, height));
	}
}
