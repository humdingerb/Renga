//////////////////////////////////////////////////
// Jabber [PeopleListItem.h]
//     A listing of an existing user
//////////////////////////////////////////////////

#ifndef PEOPLE_LIST_ITEM_H
#define PEOPLE_LIST_ITEM_H

#include <string>

#include <interface/ListItem.h>

#include <gloox/gloox.h>


class PeopleListItem : public BListItem
{
public:
	              PeopleListItem(std::string user, gloox::MUCRoomAffiliation);
	             ~PeopleListItem();

	virtual void  DrawItem(BView *owner, BRect rect, bool complete);
	virtual void  Update(BView *owner, const BFont *font);

	std::string   User() const;

private:
	std::string   _user;
	gloox::MUCRoomAffiliation fAffiliation;
};

#endif

