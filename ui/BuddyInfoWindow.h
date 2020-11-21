//////////////////////////////////////////////////
// Blabber [BuddyInfoWindow.h]
//////////////////////////////////////////////////

#pragma once

#include <interface/Box.h>
#include <interface/StringView.h>
#include <interface/Window.h>

#include "../jabber/UserID.h"

class BuddyInfoWindow : public BWindow {
public:
	                       BuddyInfoWindow(UserID *querying_user);
	                      ~BuddyInfoWindow();
};

