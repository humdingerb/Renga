//////////////////////////////////////////////////
// Blabber [MessagesPreferencesView.cpp]
//////////////////////////////////////////////////

#include "SoundPreferencesView.h"

#include <cstdio>

#include <storage/Path.h>

#include "../support/AppLocation.h"
#include "BlabberSettings.h"
#include "FileItem.h"
#include "JabberSpeak.h"
#include "Messages.h"
#include "../ui/ModalAlertFactory.h"
#include "SoundSystem.h"

SoundPreferencesView::SoundPreferencesView(BRect frame)
	: BView (frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW) {
	_surrounding                  = NULL;
	_groupchat_sounds             = NULL;
	
	SetViewColor(216, 216, 216, 255);
}

SoundPreferencesView::~SoundPreferencesView() {
}

void SoundPreferencesView::AttachedToWindow() {
	SoundSystem *sound = SoundSystem::Instance();

	// don't do it twice
	if (_surrounding) {
		return;
	}

	BRect rect(Bounds());

	// box frame
	rect.InsetBy(5.0, 5.0);
	rect.bottom = rect.top + 211.0;

	_surrounding = new BBox(rect, NULL, B_FOLLOW_ALL);
	_surrounding->SetLabel("Associated Sound Events");

	rect.OffsetBy(0.0, rect.Height() + 10.0);
	rect.bottom = rect.top + 75.0;
	
	_surrounding_options = new BBox(rect, NULL, B_FOLLOW_ALL);
	_surrounding_options->SetLabel("Other Sound Options");
	
	rect = _surrounding->Bounds();
	
	rect.InsetBy(25.0, 45.0);
	rect.right = rect.left + 320.0;

	BRect test_rect(rect);

	test_rect.left = rect.right;
	test_rect.right = test_rect.left + 35.0;
	test_rect.bottom = test_rect.top + 14.0;
	test_rect.OffsetBy(0.0, -1.0);
	
	// new chat window	
	_new_chat_selection = new BPopUpMenu(sound->NewMessageSoundLeaf().c_str());
	_new_chat_field = new BMenuField(rect, NULL, "New message window: ", _new_chat_selection);	
	_new_chat_field->SetDivider(_new_chat_field->Divider() - 24);
	
	if (!sound->NewMessageSound().empty()) {
		FileItem *new_item = new FileItem(sound->NewMessageSoundLeaf().c_str(), sound->NewMessageSound().c_str(), new BMessage(JAB_SELECTED_NEW_MESSAGE_SOUND));
		_new_chat_selection->AddItem(new_item);
		new_item->SetMarked(true);
	}
	
	_new_chat_selection->AddSeparatorItem();
	_new_chat_selection->AddItem(new BMenuItem("<none>", new BMessage(JAB_NO_NEW_MESSAGE_SOUND)));
	_new_chat_selection->AddItem(new BMenuItem("Other...", new BMessage(JAB_PICK_NEW_MESSAGE_SOUND)));

	if (sound->NewMessageSound().empty()) {
		_new_chat_selection->FindItem("<none>")->SetMarked(true);
	}

	_old_new_message_item = _new_chat_selection->FindMarked();

	_new_chat_selection->SetTargetForItems(this);

	_test_new_chat = new BButton(test_rect, NULL, "Test", new BMessage(TEST_NEW_CHAT));
	_test_new_chat->SetTarget(this);

	// incoming message window	
	rect.OffsetBy(0.0, 28.0);
	test_rect.OffsetBy(0.0, 28.0);

	_message_selection = new BPopUpMenu(sound->MessageSoundLeaf().c_str());
	_message_field = new BMenuField(rect, NULL, "Incoming message: ", _message_selection);	
	_message_field->SetDivider(_message_field->Divider() - 24);
	
	if (!sound->MessageSound().empty()) {
		FileItem *new_item = new FileItem(sound->MessageSoundLeaf().c_str(), sound->NewMessageSound().c_str(), new BMessage(JAB_SELECTED_MESSAGE_SOUND));
		_message_selection->AddItem(new_item);
		new_item->SetMarked(true);
	}
	
	_message_selection->AddSeparatorItem();
	_message_selection->AddItem(new BMenuItem("<none>", new BMessage(JAB_NO_MESSAGE_SOUND)));
	_message_selection->AddItem(new BMenuItem("Other...", new BMessage(JAB_PICK_MESSAGE_SOUND)));

	if (sound->MessageSound().empty()) {
		_message_selection->FindItem("<none>")->SetMarked(true);
	}

	_old_message_item = _message_selection->FindMarked();

	_message_selection->SetTargetForItems(this);

	_test_message = new BButton(test_rect, NULL, "Test", new BMessage(TEST_MESSAGE));
	_test_message->SetTarget(this);
	
	// new chat window	
	rect.OffsetBy(0.0, 28.0);
	test_rect.OffsetBy(0.0, 28.0);

	_now_online_selection = new BPopUpMenu(sound->UserOnlineSoundLeaf().c_str());
	_now_online_field = new BMenuField(rect, NULL, "Buddy comes online: ", _now_online_selection);	
	_now_online_field->SetDivider(_now_online_field->Divider() - 24);
	
	if (!sound->UserOnlineSoundLeaf().empty()) {
		FileItem *new_item = new FileItem(sound->UserOnlineSoundLeaf().c_str(), sound->UserOnlineSound().c_str(), new BMessage(JAB_SELECTED_USER_ONLINE_SOUND));
		_now_online_selection->AddItem(new_item);
		new_item->SetMarked(true);
	}
	
	_now_online_selection->AddSeparatorItem();
	_now_online_selection->AddItem(new BMenuItem("<none>", new BMessage(JAB_NO_USER_ONLINE_SOUND)));
	_now_online_selection->AddItem(new BMenuItem("Other...", new BMessage(JAB_PICK_USER_ONLINE_SOUND)));

	if (sound->UserOnlineSound().empty()) {
		_now_online_selection->FindItem("<none>")->SetMarked(true);
	}

	_old_user_online_item = _now_online_selection->FindMarked();

	_now_online_selection->SetTargetForItems(this);

	_test_online = new BButton(test_rect, NULL, "Test", new BMessage(TEST_ONLINE));
	_test_online->SetTarget(this);

	// new chat window	
	rect.OffsetBy(0.0, 28.0);
	test_rect.OffsetBy(0.0, 28.0);

	_now_offline_selection = new BPopUpMenu(sound->UserOfflineSoundLeaf().c_str());
	_now_offline_field = new BMenuField(rect, NULL, "Buddy goes offline: ", _now_offline_selection);	
	_now_offline_field->SetDivider(_now_offline_field->Divider() - 24);
	
	if (!sound->UserOfflineSoundLeaf().empty()) {
		FileItem *new_item = new FileItem(sound->UserOfflineSoundLeaf().c_str(), sound->UserOfflineSound().c_str(), new BMessage(JAB_SELECTED_USER_OFFLINE_SOUND));
		_now_offline_selection->AddItem(new_item);
		new_item->SetMarked(true);
	}
	
	_now_offline_selection->AddSeparatorItem();
	_now_offline_selection->AddItem(new BMenuItem("<none>", new BMessage(JAB_NO_USER_OFFLINE_SOUND)));
	_now_offline_selection->AddItem(new BMenuItem("Other...", new BMessage(JAB_PICK_USER_OFFLINE_SOUND)));

	if (sound->UserOfflineSound().empty()) {
		_now_offline_selection->FindItem("<none>")->SetMarked(true);
	}

	_old_user_offline_item = _now_offline_selection->FindMarked();

	_now_offline_selection->SetTargetForItems(this);

	_test_offline = new BButton(test_rect, NULL, "Test", new BMessage(TEST_OFFLINE));
	_test_offline->SetTarget(this);

	// new chat window	
	rect.OffsetBy(0.0, 28.0);
	test_rect.OffsetBy(0.0, 28.0);

	_alert_selection = new BPopUpMenu(sound->AlertSoundLeaf().c_str());
	_alert_field = new BMenuField(rect, NULL, "Message alerts: ", _alert_selection);	
	_alert_field->SetDivider(_alert_field->Divider() - 24);
	
	if (!sound->AlertSoundLeaf().empty()) {
		FileItem *new_item = new FileItem(sound->AlertSoundLeaf().c_str(), sound->AlertSound().c_str(), new BMessage(JAB_SELECTED_ALERT_SOUND));
		_alert_selection->AddItem(new_item);
		new_item->SetMarked(true);
	}
	
	_alert_selection->AddSeparatorItem();
	_alert_selection->AddItem(new BMenuItem("<none>", new BMessage(JAB_NO_ALERT_SOUND)));
	_alert_selection->AddItem(new BMenuItem("Other...", new BMessage(JAB_PICK_ALERT_SOUND)));

	if (sound->AlertSound().empty()) {
		_alert_selection->FindItem("<none>")->SetMarked(true);
	}

	_old_alert_item = _alert_selection->FindMarked();

	_alert_selection->SetTargetForItems(this);

	_test_alert = new BButton(test_rect, NULL, "Test", new BMessage(TEST_ALERT));
	_test_alert->SetTarget(this);

	rect = _surrounding_options->Bounds();
	rect.OffsetTo(B_ORIGIN);
	
	rect.InsetBy(25.0, 30.0);
	rect.right = rect.left + 320.0;
	rect.bottom = rect.top + 18.0;

	// groupchat message sounds?
	_groupchat_sounds = new BCheckBox(rect, NULL, "Exclude groupchat from message sound events", NULL);
	_groupchat_sounds->SetValue(BlabberSettings::Instance()->Tag("exclude-groupchat-sounds"));
	
	// children
	_surrounding->AddChild(_message_field);
	_surrounding->AddChild(_new_chat_field);
	_surrounding->AddChild(_test_new_chat);
	_surrounding->AddChild(_test_message);
	_surrounding->AddChild(_test_online);
	_surrounding->AddChild(_test_offline);
	_surrounding->AddChild(_test_alert);
	_surrounding->AddChild(_now_online_field);
	_surrounding->AddChild(_now_offline_field);
	_surrounding->AddChild(_alert_field);

	_surrounding_options->AddChild(_groupchat_sounds);
	
	AddChild(_surrounding);
	AddChild(_surrounding_options);
}

void SoundPreferencesView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case B_CANCEL: {
			uint32 what_was_i = msg->FindInt32("old_what");

			if (what_was_i == NEW_MESSAGE_FILE_OPEN) {
				_old_new_message_item->SetMarked(true);
			} else if (what_was_i == MESSAGE_FILE_OPEN) {
				_old_message_item->SetMarked(true);
			} else if (what_was_i == USER_ONLINE_FILE_OPEN) {
				_old_user_online_item->SetMarked(true);
			} else if (what_was_i == USER_OFFLINE_FILE_OPEN) {
				_old_user_offline_item->SetMarked(true);
			} else if (what_was_i == ALERT_FILE_OPEN) {
				_old_alert_item->SetMarked(true);
			}
			
			break;
		}
		
		case JAB_PICK_NEW_MESSAGE_SOUND: {
			// just open file panel for now
			entry_ref sounds;
			
			get_ref_for_path(AppLocation::Instance()->AbsolutePath("resources/sounds").c_str(), &sounds);

			_fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this, Window()), &sounds, 0, false, new BMessage(NEW_MESSAGE_FILE_OPEN));
			_fp->Show();
			
			break;
		}

		case JAB_NO_NEW_MESSAGE_SOUND: {
			_old_new_message_item = _new_chat_selection->FindMarked();

			SoundSystem::Instance()->SetNewMessageSound("<none>");
			break;
		}

		case JAB_SELECTED_NEW_MESSAGE_SOUND: {
			_old_new_message_item = _new_chat_selection->FindMarked();

			SoundSystem::Instance()->SetNewMessageSound(dynamic_cast<FileItem *>(_old_new_message_item)->Filename());
			break;
		}
		
		case NEW_MESSAGE_FILE_OPEN: {
			entry_ref file;
			string filename;

			msg->FindRef("refs", &file);
			BEntry ent(&file);
			
			BPath path;
			ent.GetPath(&path);

			filename = path.Path();			

			SoundSystem::Instance()->SetNewMessageSound(filename);
			
			// add to menu
			FileItem *new_item = new FileItem(path.Leaf(), path.Path(), new BMessage(JAB_SELECTED_NEW_MESSAGE_SOUND));
			_new_chat_selection->AddItem(new_item, 0);
			new_item->SetMarked(true);
			new_item->SetTarget(this);
			
			_old_new_message_item = _new_chat_selection->FindMarked();
			
			break;
		}

		case JAB_PICK_MESSAGE_SOUND: {
			// just open file panel for now
			entry_ref sounds;
			
			get_ref_for_path(AppLocation::Instance()->AbsolutePath("resources/sounds").c_str(), &sounds);

			_fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this, Window()), &sounds, 0, false, new BMessage(MESSAGE_FILE_OPEN));
			_fp->Show();
			
			break;
		}

		case JAB_NO_MESSAGE_SOUND: {
			_old_message_item = _message_selection->FindMarked();

			SoundSystem::Instance()->SetMessageSound("<none>");
			break;
		}

		case JAB_SELECTED_MESSAGE_SOUND: {
			_old_message_item = _message_selection->FindMarked();

			SoundSystem::Instance()->SetMessageSound(dynamic_cast<FileItem *>(_old_message_item)->Filename());
			break;
		}
		
		case MESSAGE_FILE_OPEN: {
			entry_ref file;
			string filename;

			msg->FindRef("refs", &file);
			BEntry ent(&file);
			
			BPath path;
			ent.GetPath(&path);

			filename = path.Path();			

			SoundSystem::Instance()->SetMessageSound(filename);
			
			// add to menu
			FileItem *new_item = new FileItem(path.Leaf(), path.Path(), new BMessage(JAB_SELECTED_MESSAGE_SOUND));
			_message_selection->AddItem(new_item, 0);
			new_item->SetMarked(true);
			new_item->SetTarget(this);
			
			_old_message_item = _message_selection->FindMarked();
			
			break;
		}

		case JAB_PICK_USER_ONLINE_SOUND: {
			// just open file panel for now
			entry_ref sounds;
			
			get_ref_for_path(AppLocation::Instance()->AbsolutePath("resources/sounds").c_str(), &sounds);

			_fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this, Window()), &sounds, 0, false, new BMessage(USER_ONLINE_FILE_OPEN));
			_fp->Show();
			
			break;
		}

		case JAB_NO_USER_ONLINE_SOUND: {
			_old_user_online_item = _now_online_selection->FindMarked();

			SoundSystem::Instance()->SetUserOnlineSound("<none>");
			break;
		}

		case JAB_SELECTED_USER_ONLINE_SOUND: {
			_old_user_online_item = _now_online_selection->FindMarked();

			SoundSystem::Instance()->SetUserOnlineSound(dynamic_cast<FileItem *>(_old_user_online_item)->Filename());
			break;
		}

		case USER_ONLINE_FILE_OPEN: {
			entry_ref file;
			string filename;

			msg->FindRef("refs", &file);
			BEntry ent(&file);
			
			BPath path;
			ent.GetPath(&path);

			filename = path.Path();			

			SoundSystem::Instance()->SetUserOnlineSound(filename);
			
			// add to menu
			FileItem *new_item = new FileItem(path.Leaf(), path.Path(), new BMessage(JAB_SELECTED_USER_ONLINE_SOUND));
			_now_online_selection->AddItem(new_item, 0);
			new_item->SetMarked(true);
			new_item->SetTarget(this);

			_old_user_online_item = _now_online_selection->FindMarked();
			
			break;
		}

		case JAB_PICK_USER_OFFLINE_SOUND: {
			// just open file panel for now
			entry_ref sounds;
			
			get_ref_for_path(AppLocation::Instance()->AbsolutePath("resources/sounds").c_str(), &sounds);

			_fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this, Window()), &sounds, 0, false, new BMessage(USER_OFFLINE_FILE_OPEN));
			_fp->Show();
			
			break;
		}

		case JAB_NO_USER_OFFLINE_SOUND: {
			_old_user_offline_item = _now_offline_selection->FindMarked();

			SoundSystem::Instance()->SetUserOfflineSound("<none>");
			break;
		}

		case JAB_SELECTED_USER_OFFLINE_SOUND: {
			_old_user_offline_item = _now_offline_selection->FindMarked();

			SoundSystem::Instance()->SetUserOfflineSound(dynamic_cast<FileItem *>(_old_user_offline_item)->Filename());
			break;
		}

		case USER_OFFLINE_FILE_OPEN: {
			entry_ref file;
			string filename;

			msg->FindRef("refs", &file);
			BEntry ent(&file);
			
			BPath path;
			ent.GetPath(&path);

			filename = path.Path();			

			SoundSystem::Instance()->SetUserOfflineSound(filename);
			
			// add to menu
			FileItem *new_item = new FileItem(path.Leaf(), path.Path(), new BMessage(JAB_SELECTED_USER_OFFLINE_SOUND));
			_now_offline_selection->AddItem(new_item, 0);
			new_item->SetMarked(true);
			new_item->SetTarget(this);

			_old_user_offline_item = _now_offline_selection->FindMarked();
			
			break;
		}

		case JAB_PICK_ALERT_SOUND: {
			// just open file panel for now
			entry_ref sounds;
			
			get_ref_for_path(AppLocation::Instance()->AbsolutePath("resources/sounds").c_str(), &sounds);

			_fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this, Window()), &sounds, 0, false, new BMessage(ALERT_FILE_OPEN));
			_fp->Show();
			
			break;
		}

		case JAB_NO_ALERT_SOUND: {
			_old_alert_item = _alert_selection->FindMarked();

			SoundSystem::Instance()->SetAlertSound("<none>");
			break;
		}

		case JAB_SELECTED_ALERT_SOUND: {
			_old_alert_item = _alert_selection->FindMarked();

			SoundSystem::Instance()->SetAlertSound(dynamic_cast<FileItem *>(_old_alert_item)->Filename());
			break;
		}

		case ALERT_FILE_OPEN: {
			entry_ref file;
			string filename;

			msg->FindRef("refs", &file);
			BEntry ent(&file);
			
			BPath path;
			ent.GetPath(&path);

			filename = path.Path();			

			SoundSystem::Instance()->SetAlertSound(filename);
			
			// add to menu
			FileItem *new_item = new FileItem(path.Leaf(), path.Path(), new BMessage(JAB_SELECTED_ALERT_SOUND));
			_alert_selection->AddItem(new_item, 0);
			new_item->SetMarked(true);
			new_item->SetTarget(this);

			_old_alert_item = _alert_selection->FindMarked();
			
			break;
		}

		case TEST_NEW_CHAT: {
			SoundSystem::Instance()->PlayNewMessageSound();
			break;
		}

		case TEST_MESSAGE: {
			SoundSystem::Instance()->PlayMessageSound();
			break;
		}

		case TEST_ONLINE: {
			SoundSystem::Instance()->PlayUserOnlineSound();
			break;
		}

		case TEST_OFFLINE: {
			SoundSystem::Instance()->PlayUserOfflineSound();
			break;
		}

		case TEST_ALERT: {
			SoundSystem::Instance()->PlayAlertSound();
			break;
		}
	}
}

void SoundPreferencesView::UpdateFile() {
	if (_groupchat_sounds) {
		BlabberSettings::Instance()->SetTag("exclude-groupchat-sounds", _groupchat_sounds->Value());
	}
}
