//////////////////////////////////////////////////
// Blabber [BlabberApp.cpp]
//////////////////////////////////////////////////

#include <cstdlib>

#include <InterfaceKit.h>

#include "BlabberApp.h"

#include "AboutWindow.h"
#include "BlabberMainWindow.h"
#include "BlabberSettings.h"
#include "JabberSpeak.h"
#include "MessageRepeater.h"
#include "../ui/ModalAlertFactory.h"

const char* kAppMIMEType = "application/x-vnd.Haiku-Jabber";

BlabberApp::BlabberApp()
	: BApplication(kAppMIMEType)
{
	// launch the message repeater looper
	MessageRepeater::Instance()->Run();

	// display opening window
	BlabberMainWindow::Instance()->Show();

	// launch initial login process
	JabberSpeak::Instance()->SendConnect();
}

BlabberApp::~BlabberApp() {
	// take down the message repeater
	MessageRepeater::Instance()->Lock();
	MessageRepeater::Instance()->Quit();

	// take down the network queue
	delete JabberSpeak::Instance();
	
	// clean up the settings module
	delete BlabberSettings::Instance();
}


void
BlabberApp::AboutRequested()
{
	AboutWindow::Instance()->Show();
	AboutWindow::Instance()->Activate();
}
