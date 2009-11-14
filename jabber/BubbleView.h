//////////////////////////////////////////////////
// Shared Interface [BubbleView.h]
//     Displays some text
//////////////////////////////////////////////////

#ifndef BUBBLE_VIEW_H
#define BUBBLE_VIEW_H

#ifndef _STRING_VIEW_H
	#include <interface/StringView.h>
#endif

class BubbleView : public BStringView {
public:
	 BubbleView(BRect frame, const char *name, const char *text, rgb_color bg, rgb_color fg);
	~BubbleView();

	void Draw(BRect update);
	
private:
	rgb_color _bg;
	rgb_color _fg;
};

#endif