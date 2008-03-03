//////////////////////////////////////////////////
// Blabber [BetterTextView.h]
//////////////////////////////////////////////////

#ifndef BETTER_TEXT_VIEW_H
#define BETTER_TEXT_VIEW_H

#ifndef _TEXT_VIEW_H
	#include <interface/TextView.h>
#endif

class BetterTextView : public BTextView {
public:
	BetterTextView(BRect frame, const char *name, BRect text_rect, uint32 resizing_mode, uint32 flags);
	BetterTextView(BRect frame, const char *name, BRect text_rect, const BFont *font, const rgb_color *color, uint32 resizing_mode, uint32 flags);

	void FrameResized(float width, float height);
	void KeyDown(const char *bytes, int32 numbytes);
};

#endif