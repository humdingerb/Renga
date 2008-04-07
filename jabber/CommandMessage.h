//////////////////////////////////////////////////
// Blabber [CommandMessage.h]
//     Recognizes and converts command-based
//     messages into expanded messages.
//////////////////////////////////////////////////

#ifndef COMMAND_MESSAGE_H
#define COMMAND_MESSAGE_H

#include <string>

class CommandMessage {
public:
	enum                 command_type {NOT_A_COMMAND, BAD_SYNTAX, NORMAL, NORMAL_ALERT};

public:
	static bool          IsCommand(std::string message);
	static bool          IsLegalCommand(std::string message);
	static bool          IsBadCommandSyntax(std::string message);

	static command_type  ConvertCommandToMessage(std::string &message, std::string username = "");
};

#endif
