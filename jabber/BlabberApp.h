//////////////////////////////////////////////////
// Blabber [BlabberApp.h]
//     Application control for BeOS.
//////////////////////////////////////////////////

#ifndef BLABBER_APP_H
#define BLABBER_APP_H

#ifndef _APPLICATION_H
	#include <app/Application.h>
#endif

#define APP_VERSION "v1.2.1"

class BlabberMainWindow;

class BlabberApp : public BApplication {
public:
	 BlabberApp();
	~BlabberApp();

	void AboutRequested() override;
	
private:
	 BlabberMainWindow *_blabber_main_window;
};

#endif
