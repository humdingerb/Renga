/*
 * GlooxHandler.h
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

/** @file
 *
 * Run gloox client in a thread and convert all callbacks into asynchronous
 * StartWatching/SendNotices for ease of use with the Be API.
 */

#pragma once

#include <Handler.h>

#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/registration.h>
#include <gloox/registrationhandler.h>


enum {
	kRegistrationFields = 'Rfld',
	kAlreadyRegistered = 'Ralr',
	kRegistrationResult = 'Rres',
	kDataForm = 'Rdtf',
	kOOB = 'Roob',

	kConnect = 'Ccon',
	kDisconnect = 'Cdis',
	kTLSConnect = 'Ctls'
};


class GlooxHandler: public BHandler, public gloox::RegistrationHandler,
	public gloox::ConnectionListener
{
public:
	GlooxHandler(gloox::Client* client);

	void Run();

private:
	static int32 GlooxThread(void*);

	// gloox::RegistrationHandler
	void handleRegistrationFields(const gloox::JID&, int, std::string) final;
	void handleAlreadyRegistered(const gloox::JID&) final;
	void handleRegistrationResult(const gloox::JID&, gloox::RegistrationResult) final;
	void handleDataForm(const gloox::JID&, const gloox::DataForm&) final;
	void handleOOB(const gloox::JID&, const gloox::OOB&) final;

	// gloox::ConnectionListener
	void onConnect() final;
	void onDisconnect(gloox::ConnectionError) final;
	bool onTLSConnect(const gloox::CertInfo& info) final;

private:
	gloox::Client* fClient;
	gloox::Registration* fRegistration;
};

