//////////////////////////////////////////////////
// Blabber [ChatWidget.h]
//     The widget for displaying talk sessions,
//////////////////////////////////////////////////

#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#ifndef _LIST_VIEW_H
	#include <interface/ListView.h>
#endif

#ifndef BLABBER_MAIN_WINDOW_H
	#include "BlabberMainWindow.h"
#endif

class ChatWidget : public BListView {
	public:
   	     ChatWidget(BRect frame, const char *name, list_view_type type, uint32 flags);
        ~ChatWidget();
};

#endif