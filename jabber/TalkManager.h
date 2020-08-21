//////////////////////////////////////////////////
// Blabber [TalkManager.h]
//     Handles the simultaneous talk sessions
//     going on.
//////////////////////////////////////////////////

#ifndef TALK_MANAGER_H
#define TALK_MANAGER_H

#include <gloox/messagesessionhandler.h>
#include <gloox/message.h>
#include <gloox/mucroomhandler.h>

#include <map>
#include <string>

#include "GenericFunctions.h"

#include "ui/TalkView.h"

enum {
	kWindowList = 'winl'
};

// FIXME could we replace this with MessageSessionHandler?
class TalkManager : public BHandler, public gloox::MessageSessionHandler, gloox::MUCRoomHandler, gloox::MessageHandler {
public:
	typedef  std::map<gloox::MessageSession*, TalkView *>        TalkMap;
	typedef  std::map<gloox::MUCRoom*, TalkView *>        GroupMap;

public:
	static TalkManager  *Instance();
      	                ~TalkManager();

						// TODO split out into CreateTalkSession / CreateGroupSession
	TalkView*			CreateTalkSession(const gloox::Message::MessageType type,
							const gloox::JID* user,
							std::string group_room, std::string group_username, 
							gloox::MessageSession* session, bool sound_on_new = false);

	void*                IsExistingWindowToGroup(std::string group_room);
	void                 UpdateWindowTitles(const gloox::JID& user, BString newTitle);
	void                 RemoveWindow(TalkView* view);

	void                 Reset();
	
	// MUC handlers from gloox
	void				handleMUCParticipantPresence(gloox::MUCRoom *room,
							const gloox::MUCRoomParticipant participant,
							const gloox::Presence &presence);
	void				handleMUCMessage(gloox::MUCRoom *room,
							const gloox::Message &msg, bool priv);
	bool				handleMUCRoomCreation(gloox::MUCRoom *room);
	void				handleMUCSubject(gloox::MUCRoom *room,
							const std::string &nick,
							const std::string &subject);
	void				handleMUCInviteDecline(gloox::MUCRoom *room,
							const gloox::JID &invitee,
							const std::string &reason);
	void				handleMUCError(gloox::MUCRoom *room,
							gloox::StanzaError error);
	void				handleMUCInfo(gloox::MUCRoom *room, int features,
							const std::string &name,
							const gloox::DataForm *infoForm);
	void				handleMUCItems(gloox::MUCRoom *room,
							const gloox::Disco::ItemList &items);

	// MessageSessionHandler hooks
	void handleMessageSession(gloox::MessageSession* session) final;

	// gloox::MessageHandler hooks
	void handleMessage(const gloox::Message& msg, gloox::MessageSession* session) final;
protected:
 	                     TalkManager();

private:
	static TalkManager *_instance;

	// FIXME remove
	TalkMap             fTalkMap;
	GroupMap            fGroupMap;
};

#endif

