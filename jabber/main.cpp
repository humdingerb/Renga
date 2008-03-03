//////////////////////////////////////////////////
// Blabber [main.cpp]
//////////////////////////////////////////////////

#ifndef APP_LOCATION_H
	#include "AppLocation.h"
#endif

#ifndef BLABBER_APP_H
	#include "BlabberApp.h"
#endif

int main(int argc, char **argv) {
	// record app location
	AppLocation::Instance()->SetExecutableCall(argv[0]);
	
	// create application
	BlabberApp *blabber = new BlabberApp();
	
	// start application
	blabber->Run();	

	// success on exit
	return 0;
}