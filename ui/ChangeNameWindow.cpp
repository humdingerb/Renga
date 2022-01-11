//////////////////////////////////////////////////
// Blabber [ChangeNameWindow.cpp]
//////////////////////////////////////////////////

#include "ChangeNameWindow.h"

#include <cstdio>

#include <Button.h>
#include <Catalog.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include "../support/AppLocation.h"
#include "../jabber/BlabberSettings.h"
#include "../jabber/JabberSpeak.h"
#include "../jabber/Messages.h"
#include "../ui/ModalAlertFactory.h"
#include "../jabber/TalkManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChangeNameWindow"


ChangeNameWindow::ChangeNameWindow(const gloox::JID& changing_user, BString oldName)
	: BWindow(BRect(0, 0, 100, 100), B_TRANSLATE("Change buddy name"), B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	_changing_user(changing_user)
{
	BGroupView* full_view = new BGroupView("main-full", B_VERTICAL);
	full_view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(full_view);

	BStringView *query = new BStringView(NULL, B_TRANSLATE(
		"Specify the new \"Nickname\" you'd like to use:"));

	_handle = new BTextControl(NULL, NULL, "", NULL);

	if (BlabberSettings::Instance()->Data("last-talk-sent-to")) {
		_handle->SetText(BlabberSettings::Instance()->Data("last-talk-sent-to"));
	} else {
		_handle->SetText("somebody@jabber.org");
	}

	BButton *cancel = new BButton("cancel", B_TRANSLATE("Cancel"), new BMessage(JAB_CANCEL));
	cancel->SetTarget(this);

	BButton *ok = new BButton("ok", B_TRANSLATE("Change nickname"), new BMessage(JAB_OK));
	ok->MakeDefault(true);
	ok->SetTarget(this);

	BLayoutBuilder::Group<>(full_view)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(query)
		.Add(_handle)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancel)
			.Add(ok)
		.End()
	.End();

	// focus
	_handle->SetText(oldName);
	_handle->MakeFocus(true);

	CenterOnScreen();
}


ChangeNameWindow::~ChangeNameWindow() {
}

void ChangeNameWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		//// JAB_OK
		case JAB_OK: {
			if (!strcmp(_handle->Text(), "")) {
				ModalAlertFactory::Alert(B_TRANSLATE("You cannot erase your buddy's name. "
					"If you're trying to remove this buddy, please use the \"Remove buddy\" item "
					"on the user."), B_TRANSLATE("OK"));
				_handle->MakeFocus(true);

				return;
			}

			// re-add to roster
			JabberSpeak::Instance()->SetFriendlyName(_changing_user, _handle->Text());

			PostMessage(B_QUIT_REQUESTED);

			break;
		}

		//// JAB_CANCEL
		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	}

}
