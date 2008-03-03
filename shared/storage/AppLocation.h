//////////////////////////////////////////////////
// Application Directory [AppLocation.h]
//     Provides directory information to the
//     executable path.
//////////////////////////////////////////////////

#ifndef APP_LOCATION_H
#define APP_LOCATION_H

#ifndef __STRING__
	#include <string>
#endif

class AppLocation {
public:
	static AppLocation  *Instance();
	                    ~AppLocation();

	void                 SetExecutableCall(string executable_call);
	string               Path();
	string               AbsolutePath(string relative_path);
	
protected:
	                     AppLocation();

private:
	static AppLocation *_instance;
	string              _executable_path;
};                    

#endif