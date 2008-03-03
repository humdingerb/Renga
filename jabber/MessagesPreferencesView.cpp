//////////////////////////////////////////////////
// Blabber [MessagesPreferencesView.cpp]
//////////////////////////////////////////////////

#ifndef MESSAGES_PREFERENCES_VIEW_H
	#include "MessagesPreferencesView.h"
#endif

#ifndef __CSTDIO__
	#include <cstdio>
#endif

#ifndef BLABBER_SETTINGS_h
	#include "BlabberSettings.h"
#endif

MessagesPreferencesView::MessagesPreferencesView(BRect frame)
	: BView (frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW) {
	SetViewColor(216, 216, 216, 255);

	// initialize
	for (int i=0; i<9; ++i) {
		_message[i] = NULL;
	}
}

MessagesPreferencesView::~MessagesPreferencesView() {
}

void MessagesPreferencesView::AttachedToWindow() {
	BRect frame(Bounds());

	// box frame
	frame.InsetBy(5.0, 5.0);
	_surrounding = new BBox(frame, NULL, B_FOLLOW_ALL);
	_surrounding->SetLabel("Canned Quips");

	char buffer_label[255];
	char buffer_name[255];	
	char buffer_fire_name[255];	
	
	// message prefs
	frame.InsetBy(15.0, 30.0);
	frame.right  = frame.left + 325.0;
	frame.bottom = frame.top + 18;

	BRect box_frame(frame);
	box_frame.left = frame.right + 6.0;
	box_frame.right = box_frame.left + 15.0;

	for (int i=0; i<9; ++i) {
		sprintf(buffer_label, "Quip #%d: ", i+1);
		sprintf(buffer_name, "message-%d", i+1);
		sprintf(buffer_fire_name, "fire-%d", i+1);

		_message[i] = new BTextControl(frame, NULL, buffer_label, BlabberSettings::Instance()->Data(buffer_name), NULL);
		_message[i]->SetDivider(60.0);

		_quick_fire[i] = new BCheckBox(box_frame, NULL, NULL, NULL);
		_quick_fire[i]->SetValue(BlabberSettings::Instance()->Tag(buffer_fire_name));
		
		// move frame
		frame.OffsetBy(0.0, 22.0);
		box_frame.OffsetBy(0.0, 22.0);

		_surrounding->AddChild(_message[i]);
		_surrounding->AddChild(_quick_fire[i]);
	}

	frame.OffsetBy(45.0, 11.0);
	frame.bottom = frame.top + 50.0;

	rgb_color note = {0, 0, 0, 255};
	BFont black_9(be_plain_font);
	black_9.SetSize(9.0);

	BRect text_rect(frame);
	text_rect.OffsetTo(B_ORIGIN);
	
	BTextView *enter_note = new BTextView(frame, NULL, text_rect, &black_9, &note, B_FOLLOW_H_CENTER, B_WILL_DRAW);
	enter_note->SetViewColor(216, 216, 216, 255);
	enter_note->MakeEditable(false);
	enter_note->MakeSelectable(false);
	enter_note->SetText("Note: The checkboxes to the right of the message boxes represent quick-fire options.  With quick-fire enabled, messages are sent immediately, regardless of your currently working message.  When disabled, the message text is simply added to your current message at the cursor position.");
	
	AddChild(_surrounding);
	AddChild(enter_note);
}

void MessagesPreferencesView::UpdateFile() {
	if (_message[0] == NULL) {
		return;
	}

	char buffer_name[255];
	char buffer_fire_name[255];

	for (int i=0; i<9; ++i) {
		sprintf(buffer_name, "message-%d", i+1);
		sprintf(buffer_fire_name, "fire-%d", i+1);
		
		// make sure view was inited once
		BlabberSettings::Instance()->SetData(buffer_name, _message[i]->Text());
		BlabberSettings::Instance()->SetTag(buffer_fire_name, _quick_fire[i]->Value());
	}

	// save to file
	BlabberSettings::Instance()->WriteToFile();
}
