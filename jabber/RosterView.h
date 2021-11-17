//////////////////////////////////////////////////
// Blabber [RosterView.h]
//     A graphical view of the buddy list.
//////////////////////////////////////////////////

#ifndef ROSTER_VIEW_H
#define ROSTER_VIEW_H

#include <map>

#include <interface/ListItem.h>
#include <interface/OutlineListView.h>
#include <interface/PopUpMenu.h>

#include "ui/BookmarkItem.h"
#include "ui/RosterSuperitem.h"

#include "JRoster.h"
#include "RosterItem.h"
#include "TransportItem.h"
#include "UserID.h"

class RosterView : public BOutlineListView {
public:
	typedef std::map<BListItem *, UserID::online_status>  ItemToStatusMap;
	typedef std::map<UserID::online_status, BListItem *>  StatusToItemMap;

public:
	     	            RosterView();
		               ~RosterView();

	// sort algorithm used before outline list view used
	static int          ListComparison(const void *a, const void *b);

	void                AttachedToWindow();
	RosterItem         *CurrentItemSelection();
	BookmarkItem       *CurrentBookmarkSelection();
	void                MouseDown(BPoint point);
	void                RemoveSelected();
	void                SelectionChanged();
	void                MessageReceived(BMessage*) final;

	void                LinkUser(const UserID *added_user);
	void                LinkTransport(const UserID *added_transport);
	void                LinkBookmark(const gloox::JID& added_bookmark, BString nick);
	void                UnlinkUser(const gloox::JID& removed_user);
	void                UnlinkTransport(const gloox::JID& removed_transport);
	void                UnlinkBookmark(const gloox::JID& removed_bookmark);

	int32               FindUser(const gloox::JID& compare_user);
	int32               FindTransport(const gloox::JID& compare_transport);
	int32               FindBookmark(const gloox::JID& compare_jid);

	void                UpdatePopUpMenu();
	void                UpdateRoster();

private:
	BPopUpMenu        *_popup;

	BMenuItem         *_message_item;
	BMenuItem         *_chat_item;
	BMenuItem         *_change_user_item;
	BMenuItem         *_remove_user_item;
	BMenuItem         *_user_info_item;

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
	RosterSuperitem   *_bookmarks;
};

#endif
