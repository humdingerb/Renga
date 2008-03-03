//////////////////////////////////////////////////
// Generic [ProxyLooper.cpp]
//////////////////////////////////////////////////

#ifndef PROXY_LOOPER_H
	#include "ProxyLooper.h"
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif
#include <iostream>
ProxyLooper::ProxyLooper(BView *target_view) {
	_target_view = target_view;
}

ProxyLooper::~ProxyLooper() {
}

void ProxyLooper::MessageReceived(BMessage *msg) {
	_target_view->Window()->Lock();
	_target_view->MessageReceived(msg);
	_target_view->Window()->Unlock();
}

