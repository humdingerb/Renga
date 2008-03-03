//////////////////////////////////////////////////
// Blabber [CustomStatusWindow.h]
//     Give a custom status message and status.
//////////////////////////////////////////////////

#ifndef CUSTOM_STATUS_WINDOW_H
#define CUSTOM_STATUS_WINDOW_H

#include <InterfaceKit.h>

#ifndef _RADIO_BUTTON_H
	#include <interface/RadioButton.h>
#endif

#ifndef _TEXT_CONTROL_H
	#include <interface/TextControl.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

class CustomStatusWindow : public BWindow {
public:
	static CustomStatusWindow  *Instance();

public:


	                            CustomStatusWindow();
	                           ~CustomStatusWindow();

	void                        MessageReceived(BMessage *msg);
	
private:
	static CustomStatusWindow *_instance;
	
private:
	BBox                      *_surrounding;
	BTextControl              *_handle;
	BView                     *_full_view;

	BRadioButton              *_chat;
	BRadioButton              *_away;
	BRadioButton              *_xa;
	BRadioButton              *_dnd;	
};

#endif
