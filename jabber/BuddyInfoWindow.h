//////////////////////////////////////////////////
// Blabber [BuddyInfoWindow.h]
//////////////////////////////////////////////////

#ifndef BUDDY_INFO_WINDOW_H
#define BUDDY_INFO_WINDOW_H

#ifndef _BOX_H
	#include <interface/Box.h>
#endif

#ifndef _STRING_VIEW_H
	#include <interface/StringView.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

class BuddyInfoWindow : public BWindow {
public:
	                       BuddyInfoWindow(UserID *querying_user);
	                      ~BuddyInfoWindow();

	void                   MessageReceived(BMessage *msg);
	
private:
	BBox                 *_surrounding;
	BView                *_full_view;
};

#endif