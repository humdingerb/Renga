//////////////////////////////////////////////////
// Blabber [SoundSystem.cpp]
//////////////////////////////////////////////////

#ifndef SOUND_SYSTEM_H
	#include "SoundSystem.h"
#endif

#ifndef _ENTRY_H
	#include <storage/Entry.h>
#endif

#ifndef _PATH_H
	#include <storage/Path.h>
#endif

#ifndef APP_LOCATION_H
	#include "../support/AppLocation.h"
#endif

#ifndef BLABBER_SETTINGS_H
	#include "BlabberSettings.h"
#endif

SoundSystem *SoundSystem::_instance = NULL;

SoundSystem *SoundSystem::Instance() {
	if (_instance == NULL) {
		_instance = new SoundSystem();
	}

	return _instance;
}

SoundSystem::SoundSystem() {
	_new_message_alarm = NULL;
	_message_alarm = NULL;
	_user_online_alarm = NULL;
	_user_offline_alarm = NULL;
	_alert_alarm = NULL;

	// new message sound
	if (BlabberSettings::Instance()->Data("new-message-sound")) {
		SetNewMessageSound(BlabberSettings::Instance()->Data("new-message-sound"));
	} else {
		SetNewMessageSound(AppLocation::Instance()->AbsolutePath("resources/sounds/default/new-message.wav"));
	}

	// message sound
	if (BlabberSettings::Instance()->Data("message-sound")) {
		SetMessageSound(BlabberSettings::Instance()->Data("message-sound"));
	} else {
		SetMessageSound(AppLocation::Instance()->AbsolutePath("<none>"));
	}

	// new message sound
	if (BlabberSettings::Instance()->Data("user-online-sound")) {
		SetUserOnlineSound(BlabberSettings::Instance()->Data("user-online-sound"));
	} else {
		SetUserOnlineSound(AppLocation::Instance()->AbsolutePath("resources/sounds/default/user-online.wav"));
	}

	// new message sound
	if (BlabberSettings::Instance()->Data("user-offline-sound")) {
		SetUserOfflineSound(BlabberSettings::Instance()->Data("user-offline-sound"));
	} else {
		SetUserOfflineSound(AppLocation::Instance()->AbsolutePath("resources/sounds/default/user-offline.wav"));
	}

	// new message sound
	if (BlabberSettings::Instance()->Data("alert-sound")) {
		SetAlertSound(BlabberSettings::Instance()->Data("alert-sound"));
	} else {
		SetAlertSound(AppLocation::Instance()->AbsolutePath("resources/sounds/default/alert.wav"));
	}
}

SoundSystem::~SoundSystem() {
	_instance = NULL;
}

std::string SoundSystem::NewMessageSound() {
	if (_new_message_sound != "<none>") {
		return _new_message_sound;
	} else {
		return "";
	}
}

std::string SoundSystem::NewMessageSoundLeaf() {
	if (_new_message_sound != "<none>") {
		return BPath(_new_message_sound.c_str()).Leaf();
	} else {
		return "";
	}
}

void SoundSystem::SetNewMessageSound(std::string new_message_sound) {
	BEntry entry(new_message_sound.c_str());
	if (new_message_sound == "<none>" || !entry.Exists()) {
		new_message_sound = "<none>";
	}

	_new_message_sound = new_message_sound;

	BlabberSettings::Instance()->SetData("new-message-sound", new_message_sound.c_str());

	delete _new_message_alarm; _new_message_alarm = NULL;

	if (_new_message_sound != "<none>") {
		_new_message_alarm = new BSimpleGameSound(NewMessageSound().c_str());
	}
}

void SoundSystem::PlayNewMessageSound() {
	if (_new_message_sound == "<none>") {
		return;
	}

	if (_new_message_alarm == NULL) {
		_new_message_alarm = new BSimpleGameSound(NewMessageSound().c_str());
	}

	if (_new_message_alarm) {
		_new_message_alarm->StartPlaying();
	}
}

//

std::string SoundSystem::MessageSound() {
	if (_message_sound != "<none>") {
		return _message_sound;
	} else {
		return "";
	}
}

std::string SoundSystem::MessageSoundLeaf() {
	if (_message_sound != "<none>") {
		return BPath(_message_sound.c_str()).Leaf();
	} else {
		return "";
	}
}

void SoundSystem::SetMessageSound(std::string message_sound) {
	BEntry entry(message_sound.c_str());
	if (message_sound == "<none>" || !entry.Exists()) {
		message_sound = "<none>";
	}

	_message_sound = message_sound;

	BlabberSettings::Instance()->SetData("message-sound", _message_sound.c_str());

	delete _message_alarm; _message_alarm = NULL;

	if (_message_sound != "<none>") {
		_message_alarm = new BSimpleGameSound(MessageSound().c_str());
	}
}

void SoundSystem::PlayMessageSound() {
	if (_message_sound == "<none>") {
		return;
	}

	if (_message_alarm == NULL) {
		_message_alarm = new BSimpleGameSound(MessageSound().c_str());
	}

	if (_message_alarm) {
		_message_alarm->StartPlaying();
	}
}

std::string SoundSystem::UserOnlineSound() {
	if (_user_online_sound != "<none>") {
		return _user_online_sound;
	} else {
		return "";
	}
}

std::string SoundSystem::UserOnlineSoundLeaf() {
	if (_user_online_sound != "<none>") {
		return BPath(_user_online_sound.c_str()).Leaf();
	} else {
		return "";
	}
}

void SoundSystem::SetUserOnlineSound(std::string user_online_sound) {
	BEntry entry(user_online_sound.c_str());
	if (user_online_sound == "<none>" || !entry.Exists()) {
		user_online_sound = "<none>";
	}

	_user_online_sound = user_online_sound;

	BlabberSettings::Instance()->SetData("user-online-sound", user_online_sound.c_str());

	delete _user_online_alarm; _user_online_alarm = NULL;

	if (_user_online_sound != "<none>") {
		_user_online_alarm = new BSimpleGameSound(UserOnlineSound().c_str());
	}
}

void SoundSystem::PlayUserOnlineSound() {
	if (_user_online_sound == "<none>") {
		return;
	}

	if (_user_online_alarm == NULL) {
		_user_online_alarm = new BSimpleGameSound(UserOnlineSound().c_str());
	}

	if (_user_online_alarm && _user_online_alarm->InitCheck() == B_OK) {
		_user_online_alarm->StartPlaying();
	}
}

std::string SoundSystem::UserOfflineSound() {
	if (_user_offline_sound != "<none>") {
		return _user_offline_sound;
	} else {
		return "";
	}
}

std::string SoundSystem::UserOfflineSoundLeaf() {
	if (_user_offline_sound != "<none>") {
		return BPath(_user_offline_sound.c_str()).Leaf();
	} else {
		return "";
	}
}

void SoundSystem::SetUserOfflineSound(std::string user_offline_sound) {
	BEntry entry(user_offline_sound.c_str());
	if (user_offline_sound == "<none>" || !entry.Exists()) {
		user_offline_sound = "<none>";
	}

	_user_offline_sound = user_offline_sound;

	BlabberSettings::Instance()->SetData("user-offline-sound", user_offline_sound.c_str());

	delete _user_offline_alarm; _user_offline_alarm = NULL;

	if (_user_offline_sound != "<none>") {
		_user_offline_alarm = new BSimpleGameSound(UserOfflineSound().c_str());
	}
}

void SoundSystem::PlayUserOfflineSound() {
	if (_user_offline_sound == "<none>") {
		return;
	}

	if (_user_offline_alarm == NULL) {
		_user_offline_alarm = new BSimpleGameSound(UserOfflineSound().c_str());
	}

	if (_user_offline_alarm && _user_offline_alarm->InitCheck() == B_OK) {
		_user_offline_alarm->StartPlaying();
	}
}

std::string SoundSystem::AlertSound() {
	if (_alert_sound != "<none>") {
		return _alert_sound;
	} else {
		return "";
	}
}

std::string SoundSystem::AlertSoundLeaf() {
	if (_alert_sound != "<none>") {
		return BPath(_alert_sound.c_str()).Leaf();
	} else {
		return "";
	}
}

void SoundSystem::SetAlertSound(std::string alert_sound) {
	BEntry entry(alert_sound.c_str());
	if (alert_sound == "<none>" || !entry.Exists()) {
		alert_sound = "<none>";
	}

	_alert_sound = alert_sound;

	delete _alert_alarm; _alert_alarm = NULL;

	BlabberSettings::Instance()->SetData("alert-sound", alert_sound.c_str());

	if (_alert_sound != "<none>") {
		_alert_alarm = new BSimpleGameSound(AlertSound().c_str());
	}
}

void SoundSystem::PlayAlertSound() {
	if (_alert_sound == "<none>") {
		return;
	}

	if (_alert_alarm == NULL) {
		_alert_alarm = new BSimpleGameSound(AlertSound().c_str());
	}

	if (_alert_alarm && _alert_alarm->InitCheck() == B_OK) {
		_alert_alarm->StartPlaying();
	}
}
