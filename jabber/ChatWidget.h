//////////////////////////////////////////////////
// Blabber [ChatWidget.h]
//     The widget for displaying talk sessions,
//////////////////////////////////////////////////

#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#include <interface/ListView.h>

#include "ui/MainWindow.h"

class ChatWidget : public BListView {
	public:
   	     ChatWidget(BRect frame, const char *name, list_view_type type, uint32 flags);
        ~ChatWidget();
};

#endif
