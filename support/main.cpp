//////////////////////////////////////////////////
// Blabber [main.cpp]
//////////////////////////////////////////////////

#include "AppLocation.h"

#include "../jabber/BlabberApp.h"

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
