//////////////////////////////////////////////////
// Blabber [CommandMessage.cpp]
//////////////////////////////////////////////////

#ifndef COMMAND_MESSAGE_H
	#include "CommandMessage.h"
#endif

bool CommandMessage::IsCommand(string message) {
	return (!message.empty() && message[0] == '/');
}

bool CommandMessage::IsLegalCommand(string message) {
	command_type comm_type = ConvertCommandToMessage(message);
	
	return (comm_type == NORMAL || comm_type == NORMAL_ALERT);
}

bool CommandMessage::IsBadCommandSyntax(string message) {
	return (ConvertCommandToMessage(message) == BAD_SYNTAX);
}
	
CommandMessage::command_type CommandMessage::ConvertCommandToMessage(string &message, string username) {
	string original_message = message;

	// weed out non-commands
	if (!IsCommand(message)) {
		return NOT_A_COMMAND;
	}
	
	// parse out command
	if (original_message.substr(0, 3) == "/me") {
		if (original_message.size() <= 3) {
			message = "Syntax: /me <your message>";
			message += "\n";
			
			return BAD_SYNTAX;
		}
	
		// convert
		message = "* " + username;
		message += " ";
		message += original_message.substr(4);
		message += "\n";

		return NORMAL;
	} else if (original_message.substr(0, 6) == "/alert") {
		if (original_message.size() <= 6) {
			message = "Syntax: /alert <your message>";
			message += "\n";
			
			return BAD_SYNTAX;
		}
	
		// convert
		message = original_message.substr(7);

		return NORMAL_ALERT;
	} else {
		return NOT_A_COMMAND;
	}
}