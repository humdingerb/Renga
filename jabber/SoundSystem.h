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

	 std::string         NewMessageSound();
	 std::string         NewMessageSoundLeaf();
	 void                SetNewMessageSound(std::string new_message_sound);
	 void                PlayNewMessageSound();

	 std::string         MessageSound();
	 std::string         MessageSoundLeaf();
	 void                SetMessageSound(std::string message_sound);
	 void                PlayMessageSound();

	 std::string         UserOnlineSound();
	 std::string         UserOnlineSoundLeaf();
	 void                SetUserOnlineSound(std::string user_online_sound);
	 void                PlayUserOnlineSound();

	 std::string         UserOfflineSound();
	 std::string         UserOfflineSoundLeaf();
	 void                SetUserOfflineSound(std::string user_offline_sound);
	 void                PlayUserOfflineSound();

	 std::string         AlertSound();
	 std::string         AlertSoundLeaf();
	 void                SetAlertSound(std::string alert_sound);
	 void                PlayAlertSound();

private:
	std::string          _new_message_sound;
	std::string          _message_sound;
	std::string          _user_online_sound;
	std::string          _user_offline_sound;
	std::string          _alert_sound;

	BSimpleGameSound   *_new_message_alarm;
	BSimpleGameSound   *_message_alarm;
	BSimpleGameSound   *_user_online_alarm;
	BSimpleGameSound   *_user_offline_alarm;
	BSimpleGameSound   *_alert_alarm;

private:
	static SoundSystem *_instance;
};

#endif
