//////////////////////////////////////////////////
// Blabber [ChatTextView.h]
//     Handles MouseDown.
//////////////////////////////////////////////////

#ifndef CHAT_TEXT_VIEW_H
#define CHAT_TEXT_VIEW_H

#ifndef _TEXT_VIEW_H
	#include <interface/TextView.h>
#endif

class ChatTextView : public BTextView {
public:
	ChatTextView(const char *name, uint32 flags);
	ChatTextView(const char *name, const BFont *font, const rgb_color *color, uint32 flags);

	void MouseDown(BPoint pt);
};

#endif
