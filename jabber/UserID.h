//////////////////////////////////////////////////
// Blabber [UserID.h]
//     Encapsulates user data for any account,
//     Jabber or otherwise.
//////////////////////////////////////////////////

#ifndef USER_ID_H
#define USER_ID_H

#ifndef __STRING__
	#include <string>
#endif

class UserID {
public:
	enum online_status  {UNKNOWN, UNACCEPTED, OFFLINE, ONLINE, TRANSPORT_ONLINE};
	enum user_type      {INVALID, JABBER, AIM, ICQ, YAHOO, MSN, TRANSPORT};
	
public:
          	             UserID(string username);
          	             UserID(const UserID &copied_userid);
	virtual             ~UserID();
	
	UserID              &operator=(const UserID &rhs);

	const user_type      UserType() const;
	const string         Handle() const;
	const string         FriendlyName() const;

	const string         Ask() const;
	const online_status  OnlineStatus() const;
	const string         ExactOnlineStatus() const;
	const string         MoreExactOnlineStatus() const;
	const string         SubscriptionStatus() const;

	bool                 HaveSubscriptionTo() const;
	bool                 IsUser() const;

	// Jabber
	const string         JabberHandle() const;         // xxx@yyy
	const string         JabberCompleteHandle() const; // xxx@yyy/zzz
	const string         JabberUsername() const;       // xxx
	const string         JabberServer() const;         // yyy
	const string         JabberResource() const;       // zzz

	const string         TransportID() const;
	const string         TransportUsername() const;
	const string         TransportPassword() const;

	void                 StripJabberResource();        // xxx@yyy/zzz -> xxx@yyy
	string               WhyNotValidJabberHandle();
		
	// MANIPULATORS
	void                 SetHandle(string handle);
	void                 SetFriendlyName(string friendly_name);

	void                 SetAsk(string status);	
	void                 SetOnlineStatus(online_status status);	
	void                 SetExactOnlineStatus(string exact_status);	
	void                 SetMoreExactOnlineStatus(string exact_status);	
	void                 SetSubscriptionStatus(string status);	

private:
	void                _ProcessHandle();

	// identification
	string              _handle;
	string              _friendly_name;
	string              _service;

	string              _ask;
	online_status       _status;
	string              _exact_status;
	string              _more_exact_status;
	string              _subscription_status;
	
	// split into pieces
	user_type           _user_type;

	string              _jabber_username;
	string              _jabber_server;
	string              _jabber_resource;

	string              _transport_id;
	string              _transport_username;
	string              _transport_password;
};

#endif
