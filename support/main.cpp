//////////////////////////////////////////////////
// Blabber [main.cpp]
//////////////////////////////////////////////////

#ifndef APP_LOCATION_H
	#include "../jabber/AppLocation.h"
#endif

#ifndef BLABBER_APP_H
	#include "../jabber/BlabberApp.h"
#endif

int main(__attribute__((unused)) int argc, char **argv) {
	// record app location
	AppLocation::Instance()->SetExecutableCall(argv[0]);
	
	// create application
	BlabberApp *blabber = new BlabberApp();
	
	// start application
	blabber->Run();	

	// success on exit
	return 0;
}
