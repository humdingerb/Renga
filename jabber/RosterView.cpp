//////////////////////////////////////////////////
// Blabber [RosterView.cpp]
//////////////////////////////////////////////////

#include "RosterView.h"

#include <cstdio>

#include <MenuItem.h>
#include <TranslationUtils.h>

#include "BlabberSettings.h"
#include "ui/BuddyInfoWindow.h"
#include "JabberSpeak.h"
#include "Messages.h"
#include "../ui/ModalAlertFactory.h"
#include "SoundSystem.h"
#include "TalkManager.h"

#include <strings.h>

RosterView::RosterView()
	: BOutlineListView(NULL, B_SINGLE_SELECTION_LIST) {
	SetExplicitMinSize(BSize(StringWidth("Firstname M. Lastname") + 26, B_SIZE_UNSET));
}

RosterView::~RosterView() {
	delete _popup;

	// remember superitem statuses
	BlabberSettings::Instance()->SetTag("online-collapsed", !_online->IsExpanded());
	BlabberSettings::Instance()->SetTag("unaccepted-collapsed", !_unaccepted->IsExpanded());
	BlabberSettings::Instance()->SetTag("unknown-collapsed", !_unknown->IsExpanded());
	BlabberSettings::Instance()->SetTag("offline-collapsed", !_offline->IsExpanded());
	BlabberSettings::Instance()->SetTag("transports-collapsed", !_transports->IsExpanded());
	BlabberSettings::Instance()->SetTag("bookmarks-collapsed", !_bookmarks->IsExpanded());
	BlabberSettings::Instance()->WriteToFile();
}

int RosterView::ListComparison(const void *a, const void *b) {
	const char *str_a = (*(RosterItem **)a)->Text();
	const char *str_b = (*(RosterItem **)b)->Text();

	return strcasecmp(str_a, str_b);
}

void RosterView::AttachedToWindow() {
	// superclass call
	BOutlineListView::AttachedToWindow();

	// on double-click
	SetSelectionMessage(new BMessage(JAB_OPEN_CHAT_WITH_DOUBLE_CLICK));

	// popup menu
	_popup = new BPopUpMenu(NULL, false, false);

		_chat_item         = new BMenuItem("Chat" B_UTF8_ELLIPSIS, new BMessage(JAB_OPEN_CHAT));
		_message_item      = new BMenuItem("Send Message" B_UTF8_ELLIPSIS, new BMessage(JAB_OPEN_MESSAGE));
		_change_user_item  = new BMenuItem("Edit Buddy", new BMessage(JAB_OPEN_EDIT_BUDDY_WINDOW));
		_remove_user_item  = new BMenuItem("Remove Buddy", new BMessage(JAB_REMOVE_BUDDY));
		_user_info_item    = new BMenuItem("Get Buddy Info", new BMessage(JAB_USER_INFO));

		_presence          = new BMenu("Presence");

			_subscribe_presence   = new BMenuItem("Subscribe", new BMessage(JAB_SUBSCRIBE_PRESENCE));
			_unsubscribe_presence = new BMenuItem("Unsubscribe", new BMessage(JAB_UNSUBSCRIBE_PRESENCE));

			_presence->AddItem(_subscribe_presence);
			_presence->AddItem(_unsubscribe_presence);

	_popup->AddItem(_chat_item);
	_popup->AddItem(_message_item);
	_popup->AddSeparatorItem();
	_popup->AddItem(_change_user_item);
	_popup->AddItem(_remove_user_item);
	_popup->AddSeparatorItem();
	_popup->AddItem(_user_info_item);
	_popup->AddSeparatorItem();
	_popup->AddItem(_presence);

	// create top level lists
	AddItem(_online  = new RosterSuperitem("Online"));
	AddItem(_unaccepted = new RosterSuperitem("Pending Presence"));
	AddItem(_unknown = new RosterSuperitem("No Presence"));
	AddItem(_offline = new RosterSuperitem("Offline"));
	AddItem(_transports = new RosterSuperitem("Live Transports"));
	AddItem(_bookmarks = new RosterSuperitem("Group chats"));

	// make maps (BUGBUG better way to do two-way map?)
	_item_to_status_map[_offline] = UserID::OFFLINE;
	_item_to_status_map[_online]  = UserID::ONLINE;
	_item_to_status_map[_unknown] = UserID::UNKNOWN;
	_item_to_status_map[_unaccepted] = UserID::UNACCEPTED;
	_item_to_status_map[_transports] = UserID::TRANSPORT_ONLINE;

	// ignore online...doesn't seem to work...?
	_offline->SetExpanded(!BlabberSettings::Instance()->Tag("offline-collapsed"));
	_unknown->SetExpanded(!BlabberSettings::Instance()->Tag("unknown-collapsed"));
	_unaccepted->SetExpanded(!BlabberSettings::Instance()->Tag("unaccepted-collapsed"));
	_transports->SetExpanded(!BlabberSettings::Instance()->Tag("transports-collapsed"));
	_bookmarks->SetExpanded(!BlabberSettings::Instance()->Tag("bookmarks-collapsed"));

	_status_to_item_map[UserID::OFFLINE] = _offline;
	_status_to_item_map[UserID::ONLINE]  = _online;
	_status_to_item_map[UserID::UNKNOWN] = _unknown;
	_status_to_item_map[UserID::TRANSPORT_ONLINE] = _transports;
	_status_to_item_map[UserID::UNACCEPTED] = _unaccepted;

	// BUGBUG events
	_presence->SetTargetForItems(Window());
	_popup->SetTargetForItems(Window());

	BookmarkManager::Instance().StartWatching(this, kBookmarks);
	TalkManager::Instance()->StartWatching(this, kWindowList);
	TalkManager::Instance()->StartWatching(this, kAvatarUpdate);
}

RosterItem *RosterView::CurrentItemSelection() {
	int32 index = CurrentSelection();

	if (index >= 0) {
		return dynamic_cast<RosterItem *>(ItemAt(index));
	} else {
		return NULL;
	}
}

BookmarkItem* RosterView::CurrentBookmarkSelection()
{
	int32 index = CurrentSelection();

	if (index >= 0) {
		return dynamic_cast<BookmarkItem *>(ItemAt(index));
	} else {
		return NULL;
	}
}

void RosterView::MouseDown(BPoint point) {
	// accept first click
	Window()->Activate(true);

	// get mouse info before it's too late!
	uint32 buttons = 0;
	GetMouse(&point, &buttons, true);

	// superclass stuff
	BOutlineListView::MouseDown(point);

	if (buttons & B_SECONDARY_MOUSE_BUTTON) {
		// update menu before presentation
		UpdatePopUpMenu();

		BPoint screen_point(point);
		ConvertToScreen(&screen_point);

		BRect r(screen_point.x - 4, screen_point.y - 20, screen_point.x + 24, screen_point.y + 4);
		_popup->Go(screen_point, true, true, r, false);
		//_popup->Go(screen_point, true, true, false);
	}
}

void RosterView::RemoveSelected() {
	if (CurrentItemSelection()) {
		// numeric and object based selections
		int32       selected = CurrentSelection();
		RosterItem *item     = CurrentItemSelection();

		if (item == NULL) {
			// not a roster item, won't remove
			return;
		}

		// remove item from view
		RemoveItem(CurrentSelection());

		// select next buddy for continuity
		if (ItemAt(selected))
			Select(selected);
		else if (ItemAt(selected - 1))
			Select(selected - 1);
	}
}

void RosterView::SelectionChanged() {
	BOutlineListView::SelectionChanged();
}


void RosterView::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
		{
			int32 what = message->FindInt32("be:observe_change_what");
			switch (what) {
				case kBookmarks:
				{
					int index = 0;
					BString name;
				   	while (message->FindString("jid", index, &name) == B_OK) {
						gloox::JID jid(name.String());
						if (message->FindBool("delete", index)) {
							UnlinkBookmark(jid);
						} else {
							BString name = message->FindString("name");
							LinkBookmark(jid, name);
						}
						index++;
					}
					break;
				}
				case kWindowList:
				{
					// Update group chats status (anything below the "group chat" header)
					BRect b = Bounds();
					b.top = ItemFrame(IndexOf(_bookmarks)).bottom;
					Invalidate(b);
					break;
				}
				case kAvatarUpdate:
				{
					BString name = message->FindString("jid");
					if (name.IsEmpty()) {
						// This is for the "self" avatar
						// TODO display it somewhere?
						break;
					}
					gloox::JID jid(name.String());
					ssize_t size;
					const void* data;
					message->FindData("avatar", B_RAW_TYPE, &data, &size);
					BMemoryIO io(data, size);
					BBitmap* bitmap = BTranslationUtils::GetBitmap(&io);
					if (bitmap) {
						int32 index = FindUser(jid);
						if (index > 0) {
							dynamic_cast<RosterItem*>(FullListItemAt(index))->SetAvatar(bitmap);
							Invalidate(ItemFrame(index));
						}
					} else {
						printf("decode error for %s avatar\n", name.String());
					}
					break;
				}
				default:
				{
					message->PrintToStream();
					break;
				}
			}
			break;
		}
		default:
		{
			BOutlineListView::MessageReceived(message);
			break;
		}
	}
}


void RosterView::LinkUser(const UserID *added_user) {
	AddUnder(new RosterItem(added_user), _offline);
}

void RosterView::LinkTransport(const UserID *added_transport) {
	AddUnder(new TransportItem(added_transport), _transports);
}


static int compareStrings(const char* a, const char* b)
{
	// FIXME use ICU locale aware comparison instead
	int icompare = strcasecmp(a, b);
	if (icompare != 0)
		return icompare;

	// In case the names are case-insensitive-equal, still sort them in a
	// predictible way
	return strcmp(a, b);
}


int CompareBookmarks(const BListItem* first, const BListItem* second) {
	gloox::JID firstBookmark = dynamic_cast<const BookmarkItem*>(first)->GetUserID();
	gloox::JID secondBookmark = dynamic_cast<const BookmarkItem*>(second)->GetUserID();
	return compareStrings(firstBookmark.full().c_str(), secondBookmark.full().c_str());
}


void RosterView::LinkBookmark(const gloox::JID& added_bookmark, BString name) {
	int32 index = FindBookmark(added_bookmark);
	if (index < 0) {
		AddUnder(new BookmarkItem(added_bookmark, name), _bookmarks);
		SortItemsUnder(_bookmarks, true, &CompareBookmarks);
	} else {
		BookmarkItem* item = dynamic_cast<BookmarkItem*>(FullListItemAt(index));
		item->Update(this, be_plain_font);
		Invalidate(ItemFrame(index));
	}
}


void RosterView::UnlinkUser(const gloox::JID& removed_user) {
	// does user exist
	int32 index = FindUser(removed_user);

	if (index >= 0) {
		RemoveItem(index);
	}
}

void RosterView::UnlinkTransport(const gloox::JID& removed_transport) {
	// does transport exist
	int32 index = FindTransport(removed_transport);

	if (index >= 0) {
		RemoveItem(index);
	}
}

void RosterView::UnlinkBookmark(const gloox::JID& removed_bookmark) {
	// does transport exist
	int32 index = FindBookmark(removed_bookmark);

	if (index >= 0) {
		RemoveItem(index);
	}
}

int32 RosterView::FindUser(const gloox::JID& compare_user) {
	for (int i=0; i<FullListCountItems(); ++i) {
		// get item
		RosterItem *item = dynamic_cast<RosterItem *>(FullListItemAt(i));

		if (item == NULL || item->StalePointer()) {
			continue;
		}

		// compare against RosterView
		if (item->GetUserID()->JID().bare() == compare_user.bare()) {
			return i;
		}
	}

	// no match
	return -1;
}

int32 RosterView::FindTransport(const gloox::JID& compare_transport) {
	for (int i=0; i<FullListCountItems(); ++i) {
		// get item
		TransportItem *item = dynamic_cast<TransportItem *>(FullListItemAt(i));

		if (item == NULL) {
			continue;
		}

		// compare against RosterView
		if (item->GetUserID()->JID() == compare_transport) {
			return i;
		}
	}

	// no match
	return -1;
}


int32 RosterView::FindBookmark(const gloox::JID& compare_jid)
{
	for (int i=0; i<FullListCountItems(); ++i) {
		// get item
		BookmarkItem *item = dynamic_cast<BookmarkItem *>(FullListItemAt(i));

		if (item == NULL) {
			continue;
		}

		// compare against RosterView
		if (item->GetUserID() == compare_jid) {
			return i;
		}
	}

	// no match
	return -1;
}

void RosterView::UpdatePopUpMenu() {
	char buffer[1024];

	RosterItem *item = CurrentItemSelection();
	BookmarkItem *bookmark = CurrentBookmarkSelection();

	if (item && !item->StalePointer()) {
		const UserID *user = item->GetUserID();

		// if an item is selected
		_chat_item->SetEnabled(true);
		_message_item->SetEnabled(true);

		sprintf(buffer, "Edit %s", item->GetUserID()->FriendlyName().c_str());
		_change_user_item->SetLabel(buffer);
		_change_user_item->SetEnabled(true);

		sprintf(buffer, "Remove %s", item->GetUserID()->FriendlyName().c_str());
		_remove_user_item->SetLabel(buffer);
		_remove_user_item->SetEnabled(true);

		_user_info_item->SetEnabled(true);

		_presence->SetEnabled(true);

		if (user->HaveSubscriptionTo()) {
			_subscribe_presence->SetEnabled(false);
			_unsubscribe_presence->SetEnabled(true);
		} else {
			_subscribe_presence->SetEnabled(true);
			_unsubscribe_presence->SetEnabled(false);
		}
	} else if (bookmark) {
		// It's a bookmark/group chat, only some features are available
		_chat_item->SetEnabled(true);
		_message_item->SetEnabled(false);

		sprintf(buffer, "Edit %s", bookmark->Text());
		_change_user_item->SetLabel(buffer);
		_change_user_item->SetEnabled(true);

		sprintf(buffer, "Remove %s", bookmark->Text());
		_remove_user_item->SetLabel(buffer);
		_remove_user_item->SetEnabled(true);

		_user_info_item->SetEnabled(false);

		_presence->SetEnabled(false);
	} else {
		// if not
		_chat_item->SetEnabled(false);
		_message_item->SetEnabled(false);

		sprintf(buffer, "Edit Buddy");
		_change_user_item->SetLabel(buffer);
		_change_user_item->SetEnabled(false);

		sprintf(buffer, "Remove Buddy");
		_remove_user_item->SetLabel(buffer);
		_remove_user_item->SetEnabled(false);

		_user_info_item->SetEnabled(false);

		_presence->SetEnabled(false);
	}
}

void RosterView::UpdateRoster() {
	JRoster *roster = JRoster::Instance();

	// add entries from JRoster that are not in RosterView
	roster->Lock();

	for (JRoster::ConstRosterIter i = roster->BeginIterator(); i != roster->EndIterator(); ++i) {
		if ((*i)->IsUser() && FindUser((*i)->JID()) < 0) {
			// this entry does not exist in the RosterView
			LinkUser(*i);
		} else if ((*i)->UserType() == UserID::TRANSPORT && FindTransport((*i)->JID()) < 0 && (*i)->OnlineStatus() == UserID::TRANSPORT_ONLINE) {
			LinkTransport(*i);
		}
	}

	// adjust online status of users
	RESET:
	for (int i = 0; i < FullListCountItems(); ++i) {
		RosterItem *item = dynamic_cast<RosterItem *>(FullListItemAt(i));
		TransportItem *transport_item = dynamic_cast<TransportItem *>(FullListItemAt(i));

		// skip illegal entries
		if (item == NULL && transport_item == NULL) {
			continue;
		}

		if (item) {
			// process removals
			if (!roster->FindUser(item->GetUserID()->JID())) {
				item->SetStalePointer(true);
				RemoveItem(i);

				goto RESET;
			}

			// change of statuses
			if (item->GetUserID()->OnlineStatus() != _item_to_status_map[Superitem(item)]) {
				UserID::online_status old_status = _item_to_status_map[Superitem(item)];

				// remove the item from the current superitem...
				RemoveItem(i);

				// and add it to the appropriate one
				AddUnder(item, _status_to_item_map[item->GetUserID()->OnlineStatus()]);

				// sound effect? :)
				if (item->GetUserID()->OnlineStatus() == UserID::ONLINE && item->GetUserID()->IsUser() && old_status == UserID::OFFLINE) {
					SoundSystem::Instance()->PlayUserOnlineSound();
				} else if (item->GetUserID()->OnlineStatus() == UserID::OFFLINE && item->GetUserID()->IsUser() && old_status == UserID::ONLINE) {
					SoundSystem::Instance()->PlayUserOfflineSound();
				}

				goto RESET;
			}

			// clean it
			InvalidateItem(i);
		} else if (transport_item) {
			if (!roster->FindUser(transport_item->GetUserID()->JID()) || transport_item->GetUserID()->OnlineStatus() != UserID::TRANSPORT_ONLINE) {
				transport_item->SetStalePointer(true);
				RemoveItem(i);

				goto RESET;
			}

			// change of statuses
			if (transport_item->GetUserID()->OnlineStatus() != _item_to_status_map[Superitem(transport_item)]) {
				// remove the item from the current superitem...
				RemoveItem(i);

				// and add it to the appropriate one
				if (transport_item->GetUserID()->OnlineStatus() == UserID::TRANSPORT_ONLINE) {
					AddUnder(transport_item, _status_to_item_map[transport_item->GetUserID()->OnlineStatus()]);
				}

				// sound effect? :)
				if (transport_item->GetUserID()->OnlineStatus() == UserID::TRANSPORT_ONLINE) {
					SoundSystem::Instance()->PlayUserOnlineSound();
				} else if (transport_item->GetUserID()->OnlineStatus() == UserID::OFFLINE) {
					SoundSystem::Instance()->PlayUserOfflineSound();
				}

				goto RESET;
			}

			// clean it
			InvalidateItem(i);
		}
	}

	roster->Unlock();
}
