/*
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include "GlooxHandler.h"

#include "bob.h"
#include "media.h"
#include "../support/LogHandler.h"

#include <GridView.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <StringView.h>
#include <TextView.h>

#include <gloox/dataformitem.h>


GlooxHandler::GlooxHandler(gloox::Client* client)
	: BHandler("gloox connection")
	, fClient(client)
{
	fRegistration = new gloox::Registration(fClient);
	fRegistration->registerRegistrationHandler(this);
	fClient->registerConnectionListener(this);

	// BOB handling was in a former gloox 1.1 branch but was deleted?
	// So we have our own version, but we must register it here.
	fClient->registerStanzaExtension(new gloox::BOB());

	// And this one isn't in Gloox yet. TODO upstream it
	fClient->registerStanzaExtension(new Media());

	// TODO handle iq/query/x/field/media xmlns='urn:xmpp:media-element'

	fClient->logInstance().registerLogHandler(gloox::LogLevelDebug,
		gloox::LogAreaAll, new LogHandler());
}

void GlooxHandler::Run()
{
	thread_id thread = spawn_thread(GlooxThread, "gloox connection", B_LOW_PRIORITY, this);
	resume_thread(thread);
}


int32 GlooxHandler::GlooxThread(void* arg)
{
	GlooxHandler* object = (GlooxHandler*)arg;

	object->fClient->connect();

	return 0;
}


void GlooxHandler::handleRegistrationFields(const gloox::JID&, int, std::string)
{
	SendNotices(kRegistrationFields);
}


void GlooxHandler::handleAlreadyRegistered(const gloox::JID&)
{
	SendNotices(kAlreadyRegistered);
}


void GlooxHandler::handleRegistrationResult(const gloox::JID& from, gloox::RegistrationResult result)
{
	BMessage* message = new BMessage(kRegistrationResult);
	message->AddString("gloox::JID", from.full().c_str());
	message->AddInt32("gloox::RegistrationResult", result);
	SendNotices(kRegistrationResult, message);
}


void GlooxHandler::handleDataForm(const gloox::JID& from, const gloox::DataForm& form)
{
	BMessage* message = new BMessage(kDataForm);

	BGridView* registrationForm = new BGridView("gloox::DataForm");

	int line = 0;

	if (!form.title().empty()) {
		BStringView* titleView = new BStringView("title", form.title().c_str());
		titleView->SetFont(be_bold_font);

		BLayoutBuilder::Grid<>(registrationForm).Add(titleView, 0, line++, 3, 1);
	}

	if (!form.instructions().empty()) {
		BString instructions;
		for (const auto& line: form.instructions()) {
			instructions << line.c_str() << "\n";
		}

		BTextView* instructionsView = new BTextView("instructions");
		instructionsView->MakeEditable(false);
		instructionsView->MakeSelectable(false);
		instructionsView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
		instructionsView->SetText(instructions);
		float charSize = instructionsView->StringWidth("W");
		instructionsView->SetExplicitMinSize(BSize(charSize * 30, charSize * 7));

		BLayoutBuilder::Grid<>(registrationForm).Add(instructionsView, 0, line++, 3, 1);
	}

	bool hasRequiredFields = false;
	fprintf(stderr, "--------------\n%s\n", form.tag()->xml().c_str());

	for (auto f: form.fields()) {
		switch(f->type()) {
			case gloox::DataFormField::TypeTextSingle:
			case gloox::DataFormField::TypeTextPrivate:
			case gloox::DataFormField::TypeHidden:
			{
				BTextControl* fieldView = new BTextControl(f->name().c_str(),
					f->label().c_str(), f->value().c_str(), NULL);
				if (!f->description().empty()) {
					fieldView->SetToolTip(f->description().c_str());
				}
				BLayoutBuilder::Grid<>(registrationForm).Add(
					fieldView->CreateLabelLayoutItem(), 0, line);
				BLayoutBuilder::Grid<>(registrationForm).Add(
					fieldView->CreateTextViewLayoutItem(), 1, line);
				if (f->required()) {
					BLayoutBuilder::Grid<>(registrationForm).Add(
						new BStringView("requiredMarker", "*"), 2, line);
				}

				if (f->type() == gloox::DataFormField::TypeTextPrivate) {
					fieldView->TextView()->HideTyping(true);
				}
				if (f->type() == gloox::DataFormField::TypeHidden) {
					fieldView->Hide();
				}
				break;
			}
			case gloox::DataFormField::TypeFixed:
			{
				BStringView* fieldView = new BStringView(f->name().c_str(), f->value().c_str());
				if (!f->description().empty()) {
					fieldView->SetToolTip(f->description().c_str());
				}
				if (!f->label().empty()) {
					fprintf(stderr, "Fixed field with label\n");
				}
				BLayoutBuilder::Grid<>(registrationForm).Add(
					fieldView, 0, line, 2, 1);
				break;
			}
			default:
			{
				fprintf(stderr, "Unhandled form type %d for %s [%s] (%d)\n",
					f->type(), f->name().c_str(), f->label().c_str(), f->required());
				break;
			}
		}
		line++;
	}

	if (hasRequiredFields) {
		BLayoutBuilder::Grid<>(registrationForm).Add(
			new BStringView("footer", "* required fields"), 0, line++);
	}
#if 1
	for (const gloox::DataFormItem* i: form.items()) {
		fprintf(stderr, "-------------\n");
		for (auto f: i->fields()) {
			fprintf(stderr, "%s [%s] (%d)\n", f->name().c_str(), f->label().c_str(), f->required());
		}
	}
#endif

	registrationForm->Archive(message);
	delete registrationForm;

	message->AddString("gloox::JID", from.full().c_str());
	SendNotices(kDataForm, message);
}


void GlooxHandler::handleOOB(const gloox::JID&, const gloox::OOB&)
{
	SendNotices(kOOB);
}


void GlooxHandler::onConnect()
{
	// Ok, not very clean, the listener could tell us to do that instead...
	if (fRegistration)
		fRegistration->fetchRegistrationFields();
	SendNotices(kConnect);
}


void GlooxHandler::onDisconnect(gloox::ConnectionError error)
{
	BMessage* message = new BMessage(kDisconnect);
	message->AddInt32("gloox::ConnectionError", error);
	if (error == gloox::ConnStreamError)
		message->AddInt32("gloox::StreamError", fClient->streamError());
	SendNotices(kDisconnect, message);
}


bool GlooxHandler::onTLSConnect(const gloox::CertInfo& info __attribute__((unused)))
{
	SendNotices(kTLSConnect);
	// TODO let the listener (if any?) decide what to do
	return true;
}
