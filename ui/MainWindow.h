//////////////////////////////////////////////////
// Blabber [BlabberMainWindow.h]
//	 Central information display.
//////////////////////////////////////////////////

#pragma once

#include <Button.h>
#include <CardLayout.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>

#include "ui/PictureView.h"

#include "jabber/RosterView.h"
#include "jabber/StatusView.h"


class TalkView;

enum {
	kResetWindow = 'Wrst',
	kAddTalkView = 'Watv'
};

class BlabberMainWindow : public BWindow {
public:
	static BlabberMainWindow	*Instance();
								~BlabberMainWindow();

	virtual void				MessageReceived(BMessage *msg);
	virtual void				MenusBeginning();
	virtual bool				QuitRequested();

	bool						ValidateLogin();
	void						ShowLogin();
	void						SetCustomStatus(std::string status);


protected:
								BlabberMainWindow(BRect frame);

private:
	void						AddTalkView(TalkView* view);

private:
	// singleton
	static BlabberMainWindow *_instance;

	// GUI
	BView					*_login_full_view;

	PictureView				*_login_bulb;
	BTextControl			*_login_realname;
	BTextControl			*_login_username;
	BTextControl			*_login_password;

	BButton					*_login_new_account;
	BCheckBox				*_login_auto_login;

	BButton					*_login_login;

	BMenuItem				*_disconnect_item;
	BMenuItem				*_about_item;
	BMenuItem				*_quit_item;
	BMenuItem				*_add_buddy_item;
	BMenuItem				*_change_buddy_item;
	BMenuItem				*_remove_buddy_item;
	BMenuItem				*_user_info_item;
	BMenuItem				*_chat_item;
	BMenuItem				*_away_item;
	BMenuItem				*_dnd_item;
	BMenuItem				*_xa_item;
	BMenuItem				*_school_item;
	BMenuItem				*_work_item;
	BMenuItem				*_lunch_item;
	BMenuItem				*_dinner_item;
	BMenuItem				*_sleep_item;
	BMenuItem				*_custom_item;

	BMenuItem				*_send_message_item;
	BMenuItem				*_send_chat_item;
	BMenuItem				*_send_groupchat_item;

	BMenuItem				*_preferences_item;
	BMenuItem				*_beos_user_item;
	BMenuItem				*_riv_item;
	BMenuItem				*_jabber_org_item;
	BMenuItem				*_jabber_central_org_item;
	BMenuItem				*_jabber_view_com_item;
	BMenuItem				*_user_guide_item;
	BMenuItem				*_faq_item;

	BView					*_full_view;

	PictureView				*_title;
	StatusView				*_status_view;

	BPopUpMenu				*_online_status_selection;
	BMenuField				*_online_status;

	RosterView				*_roster;

	BCardLayout*			fTalkCards;
};
