//////////////////////////////////////////////////
// Blabber [ChatWidget.cpp]
//////////////////////////////////////////////////

#ifndef CHAT_WDGET_H
	#include "ChatWidget.h"
#endif

ChatWidget::ChatWidget(BRect frame, const char *name, list_view_type type, uint32 flags)
	: BListView(frame, name, type, flags) {
}

ChatWidget::~ChatWidget() {
}
