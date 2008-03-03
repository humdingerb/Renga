//////////////////////////////////////////////////
// Blabber [SoundSystem.h]
//     Controls sound effects.
//////////////////////////////////////////////////

#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#ifndef __STRING__
	#include <string>
#endif

#ifndef _SIMPLE_GAME_SOUND_H
	#include <game/SimpleGameSound.h>
#endif

class SoundSystem {
public:
	static SoundSystem  *Instance();
	
public:
	                     SoundSystem();
    	                ~SoundSystem();
	 
	 string              NewMessageSound();
	 string              NewMessageSoundLeaf();
	 void                SetNewMessageSound(string new_message_sound);
	 void                PlayNewMessageSound();

	 string              MessageSound();
	 string              MessageSoundLeaf();
	 void                SetMessageSound(string message_sound);
	 void                PlayMessageSound();

	 string              UserOnlineSound();
	 string              UserOnlineSoundLeaf();
	 void                SetUserOnlineSound(string user_online_sound);
	 void                PlayUserOnlineSound();

	 string              UserOfflineSound();
	 string              UserOfflineSoundLeaf();
	 void                SetUserOfflineSound(string user_offline_sound);
	 void                PlayUserOfflineSound();

	 string              AlertSound();
	 string              AlertSoundLeaf();
	 void                SetAlertSound(string alert_sound);
	 void                PlayAlertSound();
	 
private:
	string              _new_message_sound;
	string              _message_sound;
	string              _user_online_sound;
	string              _user_offline_sound;
	string              _alert_sound;
	
	BSimpleGameSound   *_new_message_alarm;
	BSimpleGameSound   *_message_alarm;
	BSimpleGameSound   *_user_online_alarm;
	BSimpleGameSound   *_user_offline_alarm;
	BSimpleGameSound   *_alert_alarm;
	
private:
	static SoundSystem *_instance;
};

#endif