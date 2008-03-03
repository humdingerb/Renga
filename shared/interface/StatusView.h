//////////////////////////////////////////////////
// Interface [StatusView.h]
//////////////////////////////////////////////////

#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H

#ifndef __STRING__
	#include <string>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

class StatusView : public BView {
public:
	               StatusView(const char *name = NULL);
	              ~StatusView();
	
	void           AttachedToWindow();
	void           Draw(BRect rect);

	void           SetMessage(string message);
	const string   Message() const;

	const float    GetHeight() const;
	
private:
	string        _current_message;
	float         _height;
	font_height   _fh;
};

#endif