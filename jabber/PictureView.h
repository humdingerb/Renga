#ifndef PICTURE_VIEW_H
#define PICTURE_VIEW_H

#include <interface/Bitmap.h>
#include <interface/View.h>

class PictureView : public BView {
public:
			  PictureView(const char *filename, BPoint point, uint32 resizing_flags = B_FOLLOW_H_CENTER, uint32 flags = B_WILL_DRAW);
   	         ~PictureView();

	void      AttachedToWindow();
	void      Draw(BRect frame);	

	bool      InitCheck();
	
private:
	BBitmap *_bitmap;
	float    _width;
	float    _height;
};

#endif