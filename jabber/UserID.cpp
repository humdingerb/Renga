//////////////////////////////////////////////////
// Blabber [UserID.cpp]
//////////////////////////////////////////////////

#include "UserID.h"

#include <cstring>

#include "Agent.h"
#include "AgentList.h"
#include "JabberSpeak.h"

#include <gloox/rosteritem.h>
#include <gloox/rostermanager.h>

UserID::UserID(gloox::JID handle) {
	// initialize values
	SetHandle(handle);

	// bare defaults
	SetOnlineStatus(UserID::UNKNOWN);
	SetExactOnlineStatus("chat");
	SetSubscriptionStatus(gloox::S10nNone);
}

UserID::UserID(const UserID &copied_userid) {
	SetHandle(copied_userid.Handle());

	SetSubscriptionStatus(copied_userid.SubscriptionStatus());
	SetOnlineStatus(copied_userid.OnlineStatus());
	SetExactOnlineStatus(copied_userid.ExactOnlineStatus());
	SetMoreExactOnlineStatus(copied_userid.MoreExactOnlineStatus());
}

UserID::~UserID() {
}

UserID &UserID::operator=(const UserID &rhs) {
	SetHandle(rhs.Handle());
	SetFriendlyName(rhs.FriendlyName());

	SetSubscriptionStatus(rhs.SubscriptionStatus());

	return *this;
}

UserID::user_type UserID::UserType() const {
	return _user_type;
}

const std::string UserID::Handle() const {
	return _handle.full();
}

const std::string UserID::FriendlyName() const {
	gloox::RosterItem* item = JabberSpeak::Instance()->GlooxClient()
		->rosterManager()->getRosterItem(_handle);
	if (item)
		return item->name();
	return "";
}

UserID::online_status UserID::OnlineStatus() const {
	if (_status == UNKNOWN && (SubscriptionStatus() == gloox::S10nNoneOut || SubscriptionStatus() == gloox::S10nNoneOutIn || SubscriptionStatus() == gloox::S10nFromOut)) {
		return UNACCEPTED;
	} else {
		return _status;
	}
}

const std::string UserID::ExactOnlineStatus() const {
	if (OnlineStatus() == ONLINE) {
		// exact status only applies for users online
		return _exact_status;
	} else {
		// user is offline, exact status is invalid
		return "";
	}
}

const std::string UserID::MoreExactOnlineStatus() const {
	if (OnlineStatus() == ONLINE) {
		// more exact status only applies for users online
		return _more_exact_status;
	} else {
		// user is offline, more exact status is invalid
		return "";
	}
}

gloox::SubscriptionType UserID::SubscriptionStatus() const {
	return _subscription_status;
}

bool UserID::HaveSubscriptionTo() const {
	return (SubscriptionStatus() == gloox::S10nTo || SubscriptionStatus() == gloox::S10nBoth);
}

bool UserID::IsUser() const {
	return (UserType() == JABBER || UserType() == ICQ);
}

const std::string UserID::JabberHandle() const {
	std::string handle;

	std::string username = JabberUsername();
	std::string server   = JabberServer();

	if (!username.empty() && !server.empty()) {
		handle = username;
		handle += '@';
		handle += server;
	}

	return handle;
}

const std::string UserID::JabberCompleteHandle() const {
	std::string complete_handle;

	// get handle
	std::string handle = JabberHandle();

	if (!handle.empty()) {
		complete_handle = handle;

		std::string resource = JabberResource();
		if (!resource.empty()) {
			complete_handle += "/" + resource;
		}
	}

	return complete_handle;
}

const std::string UserID::JabberUsername() const {
	return _handle.username();
}

const std::string UserID::JabberServer() const {
	return _handle.server();
}

const std::string UserID::JabberResource() const {
	return _handle.resource();
}

const std::string UserID::TransportID() const {
	return _transport_id;
}

const std::string UserID::TransportUsername() const {
	return _transport_username;
}

const std::string UserID::TransportPassword() const {
	return _transport_password;
}

void UserID::StripJabberResource() {
	if (WhyNotValidJabberHandle().size()) {
		_handle.setResource("");
	}
}

std::string UserID::WhyNotValidJabberHandle() {
	if (UserType() == JABBER) {
		return "";
	}

	if (_handle.username().size() == 0 || _handle.server().size() == 0) {
		return "Jabber ID must be of the form username@server[/resource].";
	}

	// verify length
	if (_handle.username().size() > 255) {
		return "Jabber ID username part must not be longer than 255 characters.";
	}

	// verify ASCII charactership of abbreviated username
	if (_handle.username().find_first_of(":@<>'\"&") != std::string::npos) {
		return "Jabber ID username part must not contain any of the following characters in the handle: @<>:'\"&";
	}

	if (!_handle) {
		return "Jabber ID could not be parsed for an unkonwn reason";
	}

	// no errors found
	return "";
}

void UserID::SetHandle(gloox::JID handle)
{
	// initialize values
	_handle = handle;

	// process based on username structure
	if (_handle.username().size() && _handle.server().size()) {
		_user_type = JABBER;
	} else {
		_user_type = INVALID;
	}
}

void UserID::SetFriendlyName(std::string friendly_name)
{
	gloox::RosterManager* manager = JabberSpeak::Instance()->GlooxClient()->rosterManager();
	gloox::RosterItem* item = manager->getRosterItem(_handle);
	if (item) {
		item->setName(friendly_name);
		manager->synchronize();
	}
}

void UserID::SetOnlineStatus(online_status status)
{
	// special value
	if (status == UNACCEPTED) {
		status = UNKNOWN;
	}

	if (_status != status) {
		_status = status;

		// transport conversion
		if (UserType() == TRANSPORT && _status == ONLINE) {
			SetOnlineStatus(TRANSPORT_ONLINE);
		}

		SetExactOnlineStatus("");
		SetMoreExactOnlineStatus("");
	}
}

void UserID::SetExactOnlineStatus(std::string exact_status) {
	// only set legal status
	if (exact_status == "away" || exact_status == "chat" || exact_status == "xa" || exact_status == "dnd") {
		_exact_status = exact_status;

		// when this changes we must reset the more exact status
		SetMoreExactOnlineStatus("");
	}
}

void UserID::SetMoreExactOnlineStatus(std::string more_exact_status) {
	// ignore certain values
	if (more_exact_status == "Autoreply" || more_exact_status == "Online") {
		return;
	}

	_more_exact_status = more_exact_status;
}

void UserID::SetSubscriptionStatus(gloox::SubscriptionType status) {
	_subscription_status = status;

	// changing subscription may change status
	if (!HaveSubscriptionTo()) {
		SetOnlineStatus(UserID::UNKNOWN);
	} else {
		if (OnlineStatus() == UserID::UNKNOWN) {
			SetOnlineStatus(UserID::OFFLINE);
		}
	}
}
