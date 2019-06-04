//////////////////////////////////////////////////
// Blabber [JRoster.h]
//     The official JRoster repository
//////////////////////////////////////////////////

#ifndef JROSTER_H
#define JROSTER_H

#ifndef __STRING__
	#include <string>
#endif

#ifndef __VECTOR__
	#include <vector>
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

class JRoster {
public:
	// search method
	enum search_method         {FRIENDLY_NAME, HANDLE, COMPLETE_HANDLE, TRANSPORT_ID};

	// shortened
	typedef std::vector<UserID *>                 RosterList;
	typedef std::vector<UserID *>::iterator       RosterIter;
	typedef std::vector<UserID *>::const_iterator ConstRosterIter;

public:
	static JRoster              *Instance();
	                            ~JRoster();

	void                         AddRosterUser(UserID *roster_user);
	void                         AddNewUser(UserID *new_user);
	void                         RemoveUser(const UserID *removed_user);
	void                         RemoveAllUsers();

	UserID                      *FindUser(search_method search_type, std::string key);
	UserID                      *FindUser(const UserID *comparing_user);
	bool                         ExistingUserObject(const UserID *comparing_user);

	void                         SetUserStatus(std::string username, UserID::online_status status);
	UserID::online_status        UserStatus(std::string username);

	ConstRosterIter              BeginIterator();
	ConstRosterIter              EndIterator();

	void                         RefreshRoster();

	void                         Lock();
	void                         Unlock();

protected:
	                             JRoster();

private:
	static JRoster             *_instance;
	RosterList                 *_roster;

	sem_id                      _roster_lock;
};

#endif
