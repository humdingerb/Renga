//////////////////////////////////////////////////
// BitmapButton [BitmapButton.h]
//     A button that displays a graphic instead
//     of a standard BButton appearance.
//////////////////////////////////////////////////

#ifndef BITMAP_BUTTON_H
#define BITMAP_BUTTON_H

#ifndef __STRING__
	#include <string>
#endif

#include <interface/Bitmap.h>
#include <interface/Button.h>

class BitmapButton : public BButton {
	public:
		             BitmapButton(BRect size, const char *name, BMessage *msg, uint32 resizing_mode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
		            ~BitmapButton();

		void         Draw(BRect update_frame);
		
		bool         SetUpState(string filename);
		bool         SetDownState(string filename);
		bool         SetDisabledState(string filename);		

	private:
		BBitmap    *_up_state;
		BBitmap    *_down_state;
		BBitmap    *_disabled_state;
};

#endif