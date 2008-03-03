//////////////////////////////////////////////////
// Blabber [DeskbarIcon.h]
//     An icon for the deskbar.
//////////////////////////////////////////////////

#ifndef DESKBAR_ICON_H
#define DESKBAR_ICON_H

#ifndef _ENTRY_H
	#include <storage/Entry.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

class _EXPORT DeskbarIcon : public BView {
public:
	static BArchivable *Instantiate(BMessage *data);

public:
                  	    DeskbarIcon();
 	                    DeskbarIcon(BMessage *msg);
	                   ~DeskbarIcon();
	
	status_t            Archive(BMessage *data, bool deep) const;
	void                Draw(BRect update);
	void                MouseDown(BPoint pt);
	
	void                AttachedToWindow();
	void                DetachedFromWindow();
	void                Init();
	
	void                SetMyID(uint32 id);
	
	void                Pulse();
	
private:
	BBitmap           *_icon;
	entry_ref          _app_ref;
	
	int                _counter;
	uint32             _my_id;
};

#endif