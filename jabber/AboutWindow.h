//////////////////////////////////////////////////
// Blabber [AboutWindow.h]
//     Quasi-reusable about window scrolling
//     credits!
//////////////////////////////////////////////////

#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#ifndef _TEXT_VIEW_H
	#include <interface/TextView.h>
#endif

#ifndef _WINDOW_H
	#include <interface/Window.h>
#endif

class AboutWindow : public BWindow {
public:
	static AboutWindow *Instance();

public:
	                     AboutWindow(BRect frame);
	                    ~AboutWindow();
	 
	void                 PopulateCredits();
 
private:
	static AboutWindow *_instance;
	
private:
	BTextView          *_credits;
};

#endif

