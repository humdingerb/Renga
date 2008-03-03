//////////////////////////////////////////////////
// Blabber [DeskbarIcon.h]
//     An icon for the deskbar.
//////////////////////////////////////////////////

#ifndef DESKBAR_ICON_H
	#include "DeskbarIcon.h"
#endif

#ifndef _APPLICATION_H
	#include <app/Application.h>
#endif

#ifndef _APP_FILE_INFO_H
	#include <storage/AppFileInfo.h>
#endif

#ifndef _BITMAP_H
	#include <interface/Bitmap.h>
#endif

#ifndef _DESKBAR_H
	#include <be_apps/Deskbar/Deskbar.h>
#endif

#ifndef _MIME_H
	#include <storage/Mime.h>
#endif

#ifndef _ROSTER_H
	#include <app/Roster.h>
#endif

#ifndef BLABBER_MAIN_WINDOW_H
	#include "BlabberMainWindow.h"
#endif

BArchivable *DeskbarIcon::Instantiate(BMessage *msg) {
//	if  (!validate_instantiation(msg, "Jabber Desktop Icon")) {
//		return NULL;
//	}
	
	return new DeskbarIcon(msg);
}

DeskbarIcon::DeskbarIcon()
	: BView(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), "Jabber Deskbar Icon", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW) {
	app_info info;

	// initialize
	_counter = 0;
	
	// read application info
	be_app->GetAppInfo(&info);

	// save info
	_app_ref = info.ref;

	// initialize
	Init();
}

DeskbarIcon::DeskbarIcon(BMessage *msg)
	: BView(msg) {
	msg->FindRef("app_ref", &_app_ref);
	Init();
}

DeskbarIcon::~DeskbarIcon() {
}
	
status_t DeskbarIcon::Archive(BMessage *msg, bool deep) const {
	status_t err;
	app_info info;
	
	// base class archiver
	err = BView::Archive(msg, deep);

	// application information
	be_app->GetAppInfo(&info);
	
	msg->AddRef("app_ref", &info.ref);
	msg->AddString("add_on", "application/jabber");
	
	return err;
}

void DeskbarIcon::Draw(BRect update) {
	if (_icon) {
		SetDrawingMode(B_OP_OVER);
		DrawBitmap(_icon, BPoint(0, 0));
	}
}

void DeskbarIcon::MouseDown(BPoint pt) {
	if (!Window()) {
		return;
	}
		
	// get mouse position
	BMessage *msg = Window()->CurrentMessage();
	
	if (!msg) {
		return;
	}
	
	// take action
	if (msg->what == B_MOUSE_DOWN) {
		uint32 buttons   = 0;
		uint32 modifiers = 0;
		
		msg->FindInt32("buttons", (int32 *)&buttons);
		msg->FindInt32("modifiers", (int32 *)&modifiers);
		
		switch(buttons) {
			case B_PRIMARY_MOUSE_BUTTON: {
				BlabberMainWindow::Instance()->Activate();
				break;
			}
		}
	}
}
	
void DeskbarIcon::AttachedToWindow() {
	BAppFileInfo appInfo;
	BFile        file;
	
	// read the application file
	file.SetTo(&_app_ref, B_READ_ONLY);
	
	appInfo.SetTo(&file);
	
	// load the application icon
	_icon = new BBitmap(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), B_CMAP8, false, false);
	
	if (appInfo.GetIcon(_icon, B_MINI_ICON) != B_OK) {
		delete _icon;
		_icon = NULL;
	}
	
	if (Parent()) {
		SetViewColor(Parent()->ViewColor());
	}
}

void DeskbarIcon::DetachedFromWindow() {
	delete _icon;
}

void DeskbarIcon::Init() {
	_icon = NULL;
}

void DeskbarIcon::SetMyID(uint32 id) {
	_my_id = id;
}

void DeskbarIcon::Pulse() {
	cout << "tick\n";
}