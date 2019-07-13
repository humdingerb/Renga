/*
 * LogHandler.cpp
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include <gloox/loghandler.h>
#include "BeDC.h"


class LogHandler: public gloox::LogHandler
{
	public:
	LogHandler() 
		: fDebugContext("Renga")
	{
	}

	private:
	void handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string &message) override {
		fDebugContext.SendFormat("%d %d %s", level, area, message.c_str());
	}

	BeDC fDebugContext;
};
