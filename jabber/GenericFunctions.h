//////////////////////////////////////////////////
// Blabber [GenericFunctions.h]
//     Generic functions performing various
//     tasks.
//////////////////////////////////////////////////

#ifndef GENERIC_FUNCTIONS_H
#define GENERIC_FUNCTIONS_H

#ifndef __STRING__
	#include <string>
#endif

class BRect;

class GenericFunctions {
public:
	static BRect  CenteredFrame(float window_width, float window_height);
	static string GenerateUniqueID();
	static string TimeStamp();
	static string GenerateNick(string username);
	static string CrushOutWhitespace(string text);
	static int    SeparateGroupSpecifiers(string text, string &room, string &server, string &user); 
};

#endif