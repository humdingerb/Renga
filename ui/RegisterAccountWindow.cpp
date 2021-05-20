/*
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include "RegisterAccountWindow.h"

#include "jabber/BlabberSettings.h"

#include "network/BobStore.h"

#include "ui/MainWindow.h"

#include "PictureView.h"

#include <Button.h>
#include <Country.h>
#include <private/netservices/Geolocation.h>
#include <LayoutBuilder.h>
#include <Rect.h>
#include <ScrollView.h>
#include <StringItem.h>
#include <TextView.h>

#include <random>


using BPrivate::Network::BGeolocation;


enum {
	kGeolocalize = 'gloc',
	kShowServerList = 'srvl',
	kSelectServer = 'sels',
	kGetUserInfo = 'gusi',
	kCreateAccount = 'crea',
};


struct ServerInfo {
	const char* hostname;
	const char* countryCode;
};

static const ServerInfo kServerInfos[] = {
	// TODO find more servers that work, try to cover more countries
	{ "jabber.otr.im", "CA" },
	{ "jabber.cz", "CZ" },
	{ "jabber.de", "DE" },
	{ "jabberes.org", "ES" },
	{ "jabber.fr", "FR" },
	{ "step.im", "JP" },
	{ "4ept.net", "NL" },
	{ "creep.im", "RU" },
	{ "jabber.today", "US" },
};


RegisterAccountWindow::RegisterAccountWindow(BHandler* target __attribute__((unused)))
	: BWindow(BRect(0, 0, 100, 100), "Create an XMPP account", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_RESIZABLE)
{
	fLayout = new BCardLayout();
	SetLayout(fLayout);

	// TODO check that we are online, otherwise there's no point in attempting
	// a registration.

	// Card 0 - Agree to be geolocalized
	BGroupView* card0 = new BGroupView(B_VERTICAL);
	AddChild(card0);
	BTextView* agree = new BTextView("agree");
	agree->SetText("In order to find a nearby server, the application will now "
		"attempt to geolocalize you. A list of wifi networks within reach of "
		"your computer will be sent to Mozilla Location Services. The "
		"resulting estimated latitude and longitude will be sent to Geonames "
		"to deduce the country you are in. If you don't want this to happen, "
		"you will have to pick a server yourself.");
	float charSize = agree->StringWidth("W");
	agree->SetExplicitMinSize(BSize(charSize * 30, charSize * 7));
	agree->MakeEditable(false);
	agree->MakeSelectable(false);
	agree->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	// TODO add hyperlinks to MLS and Geonames
	
	BButton* yes = new BButton("yes", "Yes, please", new BMessage(kGeolocalize));
	BButton* no = new BButton("no", "No thanks!", new BMessage(kShowServerList));

	BLayoutBuilder::Group<>(card0)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(agree)
		.AddGlue()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(no)
			.Add(yes)
		.End()
	.End();

	// Card 1 - Welcome message + server list
	BGroupView* card1 = new BGroupView(B_VERTICAL);
	AddChild(card1);

	fWelcome = new BTextView("welcome");
	fWelcome->SetText("Welcome to the XMPP network! We have chosen a server for "
		"you, but you can select another one if you prefer to. The server name "
		"will be part of your XMPP identifier, so you may pick one with a "
		"short and catchy name.");
	fWelcome->SetExplicitMinSize(BSize(charSize * 30, charSize * 7));
	fWelcome->MakeEditable(false);
	fWelcome->MakeSelectable(false);
	fWelcome->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fServerBox = new BTextControl("", "", NULL);

	fServerList = new BListView("server list");
	fServerList->SetSelectionMessage(new BMessage(kSelectServer));
	BScrollView* serverScroll = new BScrollView("server scroll", fServerList, 0, false, true);
	fServerList->SetExplicitMinSize(BSize(B_SIZE_UNSET, charSize * 12));

	for (auto i: kServerInfos) {
		fServerList->AddItem(new BStringItem(i.hostname));
	}

	BButton* next = new BButton("Next", "Next", new BMessage(kGetUserInfo));

	BLayoutBuilder::Group<>(card1)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fWelcome)
		.AddGrid(B_VERTICAL)
			.SetVerticalSpacing(0)
			.Add(fServerBox, 0, 0)
			.Add(serverScroll, 0, 1)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(next)
		.End()
	.End();

	// Card 2 - Waiting for server
	BGroupView* card2 = new BGroupView(B_VERTICAL);
	AddChild(card2);

	fWaitingMessage = new BStringView("wait", "Please wait, connecting to server…");
	// TODO add a BarberPole or throbber something

	BLayoutBuilder::Group<>(card2)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fWaitingMessage)
			.AddGlue()
		.End()
	.End();

	// Card 3 - Username + password (or whatever the server wants to know)
	BGroupView* card3 = new BGroupView(B_VERTICAL);
	AddChild(card3);
	fRegistrationForm = new BGridView();

#if 0
	// TODO Create all well-known fields and hide them by default
	fUsername = new BTextControl("Username", "", NULL); // TODO show @host
	fNickname = new BTextControl("Nickname", "", NULL);
	fPassword = new BTextControl("Password", "", NULL); // TODO hide typing
	fFirstName = new BTextControl("First name", "", NULL);
	fLastName = new BTextControl("Last name", "", NULL);
	fEmail = new BTextControl("E-mail address", "", NULL);
	fPostalAddress = new BTextControl("Postal address", "", NULL);
	fCity = new BTextControl("City", "", NULL);
	fState = new BTextControl("State", "", NULL);
	fZip = new BTextControl("ZIP Code", "", NULL);
	fPhoneNumber = new BTextControl("Phone number", "", NULL);
	fUrl = new BTextControl("Homepage", "", NULL);
	fDate = new BTextControl("Date", "", NULL);
	fMisc = new BTextControl("Misc.", "", NULL);
	fExtra = new BTextControl("Extra", "", NULL);
#endif

	BButton* back = new BButton("Previous", "Previous", new BMessage(kShowServerList));
	BButton* create = new BButton("create", "Register", new BMessage(kCreateAccount));

	BLayoutBuilder::Group<>(card3)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fRegistrationForm)
		.AddGroup(B_HORIZONTAL)
			.Add(back)
			.AddGlue()
			.Add(create)
		.End()
	.End();

	fLayout->SetVisibleItem((int32)0);
}


void
RegisterAccountWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kGeolocalize:
		{
#if 0 // BGeolocation::Country not yet part of Haiku
			float lat, lon;
			BGeolocation geolocation;
			BCountry country;
			if (geolocation.LocateSelf(lat, lon) == B_OK
				&& geolocation.Country(lat, lon, country) == B_OK)
			{
				for (int i = 0; i < fServerList->CountItems; i++) {
					if (strcmp(kServerInfos[i].country, country.Code()) == 0) {
						fServerList->Select(i);

						fLayout->SetVisibleItem(1);
						return;
					}
				}
			}
#endif
			/* fallthrough */
		}
		case kShowServerList:
		{
			// User did not want to be geolocalized, or no suitable server was
			// found: select at random
			if (fServerList->CurrentSelection() < 0) {
				std::random_device generator;
				std::uniform_int_distribution<int> distribution(0,
					fServerList->CountItems());
				int random = distribution(generator);
				fServerList->Select(random);
			}
			fLayout->SetVisibleItem(1);
			break;
		}

		case kSelectServer:
		{
			BStringItem* item = dynamic_cast<BStringItem*>(
				fServerList->ItemAt(fServerList->CurrentSelection()));
			if (item) {
				fServerBox->SetText(item->Text());
				fServerBox->MarkAsInvalid(false);
			} else {
				fServerBox->SetText("");
				fServerBox->MarkAsInvalid(true);
			}
			break;
		}

		case kGetUserInfo:
		{
			fLayout->SetVisibleItem(2);

			// Cleanup the registration form from previous attempts, if any
			for (int32 count = fRegistrationForm->CountChildren(); --count >= 0;)
			{
				fRegistrationForm->RemoveChild(fRegistrationForm->ChildAt(count));
			}

			// connect to server and ask which fields are required
			gloox::Client* client = new gloox::Client(fServerBox->Text());
			fConnection = new GlooxHandler(client);
			fConnection->StartWatchingAll(this);
			fConnection->Run();

			break;
		}

		case kCreateAccount:
		{
			BView* formContainer = getRegistrationView();

			if (formContainer) {
				gloox::DataForm* dataForm = new gloox::DataForm(gloox::TypeSubmit);
				for (int32 count = formContainer->CountChildren(); --count >= 0;) {
					BView* entry = formContainer->ChildAt(count);

					if (entry->Name() == BString("gloox::title")) {
						BStringView* title = dynamic_cast<BStringView*>(entry);
						dataForm->setTitle(title->Text());
					} else {
						BTextControl* c = dynamic_cast<BTextControl*>(entry);
						if (c) {
							dataForm->addField(gloox::DataFormField::TypeNone,
								c->Name(), c->Text());

							// Extract these fields for populating the main
							// window when registration completes.
							if (c->Name() == BString("username"))
								fUsername = c->Text();
							if (c->Name() == BString("password"))
								fPassword = c->Text();
						}
					}

					// TODO serialize all fields (name and value only)
				}
				fConnection->createAccount(dataForm);
			} else {
				// TODO try to locate a FixedField form view instead and use that
				fprintf(stderr, "Don't know how to create an account without a dataform, yet\n");
			}
			// see you in handleRegistrationResult!
			break;
		}

		case B_OBSERVER_NOTICE_CHANGE:
		{
			int32 orig_what = message->FindInt32("be:observe_change_what");
			switch (orig_what) {
				case kConnect:
				{
					// TODO request registration fields here instead of in GlooxHandler
					fWaitingMessage->SetText("Setting up secure connection…");
					break;
				}
				case kTLSConnect:
				{
					// TODO confirm TLS certificate, when gloox handler supports that
					fWaitingMessage->SetText("Getting registration form from server…");
					break;
				}
				case kDisconnect:
				{
					gloox::ConnectionError error = (gloox::ConnectionError)
						message->FindInt32("gloox::ConnectionError");
					gloox::StreamError streamError = (gloox::StreamError)
						message->FindInt32("gloox::StreamError");
					onDisconnect(error, streamError);
					break;
				}
				case kRegistrationFields:
				{
					// TODO handle only if there is no dataform
					break;
				}
				case kDataForm:
				{
					BString str;
					message->FindString("gloox::JID", &str);
					gloox::JID from(str.String());
					BView* view = new BView(message);
					handleDataForm(from, view);
					break;
				}
				case kMedia:
				{
					BString uri = message->FindString("uri");
					BString type = message->FindString("type");
					handleMedia(type, BUrl(uri));
					break;
				}
				case kOOB:
				{
					// TODO handle out of band data
					message->PrintToStream();
					//handleOOB(...);
					break;
				}


				case kRegistrationResult:
				{
					BString jidString = message->FindString("gloox::JID");
					gloox::JID jid(jidString.String());
					gloox::RegistrationResult result = (gloox::RegistrationResult)
						message->FindInt32("gloox::RegistrationResult");
					handleRegistrationResult(jid, result);
					break;
				}
				default:
				{
					message->PrintToStream();
					break;
				}
			}
			break;
		}
	}
}


void
RegisterAccountWindow::Show()
{
	BWindow::Show();
	CenterOnScreen();
}


#if 0
void RegisterAccountWindow::handleRegistrationFields(const gloox::JID& from __attribute__((unused)), int requiredFields,
	std::string instructions)
{
	// FIXME if there are any required fields besides username and password,
	// ask the user to fill those in. Again I need a server which does this
	// to test this, and jabber.fr doesn't.
	fprintf(stderr, "%s (%x) -> %s %s\n", instructions.c_str(), requiredFields, fUsername->Text(), fPassword->Text());
	gloox::RegistrationFields fields;
	gloox::JID jid(fUsername->Text()); // FIXME append servername
	fields.username = jid.username();
	fields.password = fPassword->Text();
	// TODO this can return an error
	fRegistration->createAccount(
		gloox::Registration::FieldUsername | gloox::Registration::FieldPassword,
		fields);
}
#endif


void RegisterAccountWindow::handleRegistrationResult(const gloox::JID&, gloox::RegistrationResult r)
{
	switch (r)
	{
		case gloox::RegistrationSuccess:
		{
			// Put login in password into configuration
			BString jid;
			jid.SetToFormat("%s@%s", fUsername.String(), fServerBox->Text());
			BlabberSettings::Instance()->SetData("last-login", fUsername);
			BlabberSettings::Instance()->SetData("last-password", fPassword);

			// Refresh main window (disconnects a running session, but that should be ok)
			BlabberMainWindow::Instance()->PostMessage(kResetWindow);

			// And finally, we can close ourselves, the registration is complete
			PostMessage(B_QUIT_REQUESTED);
			return;
		}

		case gloox::RegistrationNotAcceptable:
		{
			BView* formContainer = getRegistrationView();
			if (formContainer) {
				// Probably a required field was not filled, highlight all empty ones
				bool required = false;
				for (int32 count = formContainer->CountChildren(); --count >= 0;) {
					BView* entry = formContainer->ChildAt(count);

					if (entry->Name() == BString("gloox::requiredMarker")) {
						required = true;
						continue;
					} else {
						BTextControl* control = dynamic_cast<BTextControl*>(entry);
						if (control) {
							bool valid = (!required) || !BString(control->Text()).IsEmpty();
							control->MarkAsInvalid(!valid);
							if (!valid)
								control->SetToolTip("This field is required.");
							else
								control->SetToolTip((const char*)NULL);
						}
						required = false;
					}
				}
			}
			break;
		}

		case gloox::RegistrationConflict:
		{
			BView* formContainer = getRegistrationView();
			if (formContainer) {
				BView* v = formContainer->FindView("username");
				BTextControl* c = dynamic_cast<BTextControl*>(v);
				if (c) {
					c->MarkAsInvalid(true);
					c->SetToolTip("This username is already registered. Pick another one.");
				} else {
					fprintf(stderr, "username not found\n");
				}
			} else {
				fprintf(stderr, "registrationview not found\n");
			}
			break;
		}

		case gloox::RegistrationNotAllowed:
		{
			fWelcome->SetText("The server currently does not allow registration.\n"
				"Try another one.");
			fServerList->ItemAt(fServerList->CurrentSelection())->SetEnabled(false);
			fLayout->SetVisibleItem(1);
			break;
		}

		case gloox::RegistrationUnknownError:
		{
			// Unknown error, better try another server that works more sanely.
			fWelcome->SetText("The server did not accept our registration request.\n"
				"Try another one.");
			fServerList->ItemAt(fServerList->CurrentSelection())->SetEnabled(false);
			fLayout->SetVisibleItem(1);
			break;
		}

		default:
		{
			// TODO something went wrong, go back to the previous screen + highlight problems
			fprintf(stderr, "%s(%d)\n", __PRETTY_FUNCTION__, r);
			break;
		}
	}
}


void RegisterAccountWindow::handleDataForm(const gloox::JID&, BView* form)
{
	fRegistrationForm->AddChild(form);
	fLayout->SetVisibleItem(3);
}


void RegisterAccountWindow::handleMedia(BString type __attribute__((unused)), BUrl uri)
{
	BPositionIO* data = NULL;
	std::string storage;

	if (uri.Protocol() == "cid") {
		// Get the data from bob registry
		storage = BobStore::Instance()->Get(uri.Path().String());
		data = new BMemoryIO(storage.c_str(), storage.length());
	} else {
		fprintf(stderr, "Unhandled protocol for %s\n", uri.UrlString().String());
		// TODO get the data using UrlRoster if it can handle it
	}

	if (data) {
		PictureView* pic = new PictureView(data);
		fRegistrationForm->AddChild(pic);
	}

	delete data;
}


#if 0
void RegisterAccountWindow::handleOOB(const gloox::JID&, const gloox::OOB& oob)
{
	// TODO this will usually provide an URL for out-of-band registration.
	// If this is all we get, it means the registration process cannot be
	// completed using RegistrationFields or DataForm, so we should redirect
	// the user to the registration web page for that server.
	fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}
#endif


void RegisterAccountWindow::onDisconnect(gloox::ConnectionError error,
	gloox::StreamError streamError)
{
	fServerBox->MarkAsInvalid(true);

	switch (error)
	{
		case gloox::ConnStreamError:
		{
			switch (streamError) {
				case gloox::ConnStreamClosed:
				{
					fWelcome->SetText("The server closed the connection unexpectedly.\n");
					fServerList->ItemAt(fServerList->CurrentSelection())->SetEnabled(false);
					break;
				}
				default:
				{
					BString message;
					message.SetToFormat("Could not understand the reply from the server.\n"
							"Stream error %d", streamError);
					fWelcome->SetText(message);
					break;
				}
			}
			break;
		}
		case gloox::ConnStreamClosed:
			fWelcome->SetText("The server closed the connection unexpectedly. "
				"Maybe your internet access is too slow to use XMPP reliably.");
			break;
		case gloox::ConnStreamVersionError:
			fWelcome->SetText("The server stream version is not compatible. "
				"Try another server.");
			fServerList->ItemAt(fServerList->CurrentSelection())->SetEnabled(false);
			break;
		case gloox::ConnConnectionRefused:
			fWelcome->SetText("Could not connect to the server.\nCheck you are "
				"online, then try another one.");
			// Do not disable the selected server, allow to retry when online
			break;
		case gloox::ConnTlsFailed:
			fWelcome->SetText("Failed to setup a secure TLS communication channel. "
				"Try another server.");
			fServerList->ItemAt(fServerList->CurrentSelection())->SetEnabled(false);
			break;
		default:
			BString message;
			message.SetToFormat("Could not connect to the server (error code: %d). "
				"Try another one.", error);
			fWelcome->SetText(message);
			fServerList->ItemAt(fServerList->CurrentSelection())->SetEnabled(false);
			break;
	}
	fLayout->SetVisibleItem(1);
}


BView* RegisterAccountWindow::getRegistrationView()
{
	BView* formContainer = NULL;
	for (int32 count = fRegistrationForm->CountChildren(); --count >= 0;)
	{
		formContainer = fRegistrationForm->ChildAt(count);
		if (formContainer->Name() == BString("gloox::DataForm"))
			break;
		formContainer = nullptr;
	}

	return formContainer;
}
