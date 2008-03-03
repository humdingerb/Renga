//////////////////////////////////////////////////
// Blabber [MessagesPreferencesView.h]
//     Presents user preferences for
//     "common quips." Used as a tab view.
//////////////////////////////////////////////////

#ifndef MESSAGES_PREFERENCES_VIEW_H
#define MESSAGES_PREFERENCES_VIEW_H

#ifndef _BOX_H
	#include <interface/Box.h>
#endif

#ifndef _CHECK_BOX_H
	#include <interface/CheckBox.h>
#endif

#ifndef _TEXT_CONTROL_H
	#include <interface/TextControl.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

class MessagesPreferencesView : public BView {
public:
	         MessagesPreferencesView(BRect frame);
	virtual ~MessagesPreferencesView();

	void     AttachedToWindow();
	
	void     UpdateFile();

private:
	BBox         *_surrounding;
	BTextControl *_message[9];
	BCheckBox    *_quick_fire[9];
};

#endif