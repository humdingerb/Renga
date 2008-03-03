//////////////////////////////////////////////////
// Blabber [CommandMessage.h]
//     Recognizes and converts command-based
//     messages into expanded messages.
//////////////////////////////////////////////////

#ifndef COMMAND_MESSAGE_H
#define COMMAND_MESSAGE_H

#ifndef __STRING__
	#include <string>
#endif

class CommandMessage {
public:
	enum                 command_type {NOT_A_COMMAND, BAD_SYNTAX, NORMAL, NORMAL_ALERT};
	
public: 
	static bool          IsCommand(string message);
	static bool          IsLegalCommand(string message);
	static bool          IsBadCommandSyntax(string message);

	static command_type  ConvertCommandToMessage(string &message, string username = "");
};

#endif
