//////////////////////////////////////////////////
// Blabber [RosterView.h]
//     A graphical view of the buddy list.
//////////////////////////////////////////////////

#ifndef ROSTER_VIEW_H
#define ROSTER_VIEW_H

#ifndef __MAP__
	#include <map>
#endif

#ifndef _LIST_ITEM_H
	#include <interface/ListItem.h>
#endif

#ifndef _OUTLINE_LIST_VIEW_H
	#include <interface/OutlineListView.h>
#endif

#ifndef _POP_UP_MENU_H
	#include <interface/PopUpMenu.h>
#endif

#ifndef JROSTER_H
	#include "JRoster.h"
#endif

#ifndef ROSTER_ITEM_H
	#include "RosterItem.h"
#endif

#ifndef ROSTER_SUPERITEM_H
	#include "RosterSuperitem.h"
#endif

#ifndef TRANSPORT_ITEM_H
	#include "TransportItem.h"
#endif

#ifndef USER_ID_H
	#include "UserID.h"
#endif

#ifndef XML_ENTITY_H
	#include "XMLEntity.h"
#endif

class RosterView : public BOutlineListView {
public:
	typedef std::map<BListItem *, UserID::online_status>  ItemToStatusMap;
	typedef std::map<UserID::online_status, BListItem *>  StatusToItemMap;

public:
	     	            RosterView(BRect frame);
		               ~RosterView();

	// sort algorithm used before outline list view used
	static int          ListComparison(const void *a, const void *b);

	void                AttachedToWindow();
	RosterItem         *CurrentItemSelection();
	void                KeyDown(const char *bytes, int32 len);
	void                MouseDown(BPoint point);
	void                RemoveSelected();
	void                SelectionChanged();

	void                LinkUser(const UserID *added_user);
	void                LinkTransport(const UserID *added_transport);
	void                UnlinkUser(const UserID *removed_user);
	void                UnlinkTransport(const UserID *removed_transport);

	int32               FindUser(const UserID *compare_user);
	int32               FindTransport(const UserID *compare_transport);

	void                UpdatePopUpMenu();
	void                UpdateRoster();

private:
	BPopUpMenu        *_popup;

	BMenuItem         *_message_item;
	BMenuItem         *_chat_item;
	BMenuItem         *_change_user_item;
	BMenuItem         *_remove_user_item;
	BMenuItem         *_user_info_item;
	BMenuItem         *_user_chatlog_item;

	BMenu             *_presence;
	BMenuItem         *_subscribe_presence;
	BMenuItem         *_unsubscribe_presence;

	StatusToItemMap    _status_to_item_map;
	ItemToStatusMap    _item_to_status_map;

	RosterSuperitem   *_offline;
	RosterSuperitem   *_online;
	RosterSuperitem   *_unknown;
	RosterSuperitem   *_unaccepted;
	RosterSuperitem   *_transports;
};

#endif
