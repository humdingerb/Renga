//////////////////////////////////////////////////
// Jabber [PeopleListItem.h]
//     A listing of an existing user
//////////////////////////////////////////////////

#ifndef PEOPLE_LIST_ITEM_H
#define PEOPLE_LIST_ITEM_H

#ifndef __STRING__
	#include <string>
#endif

#ifndef LIST_ITEM_H
	#include <interface/ListItem.h>
#endif

class PeopleListItem : public BListItem {
public:
	              PeopleListItem(string whoami, string user);
   	             ~PeopleListItem();

	virtual void  DrawItem(BView *owner, BRect rect, bool complete);
	virtual void  Update(BView *owner, const BFont *font);

	string        User() const;
	
private:
	string       _user;
	string       _whoami;
}; 

#endif

