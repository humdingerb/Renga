//////////////////////////////////////////////////
// Blabber [BlabberApp.h]
//     Application control for BeOS.
//////////////////////////////////////////////////

#ifndef BLABBER_APP_H
#define BLABBER_APP_H

#ifndef _APPLICATION_H
	#include <app/Application.h>
#endif

class BlabberMainWindow;

class BlabberApp : public BApplication {
public:
	 BlabberApp();
	~BlabberApp();
	
private:
	 BlabberMainWindow *_blabber_main_window;
};

#endif