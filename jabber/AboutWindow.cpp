//////////////////////////////////////////////////
// Blabber [AboutWindow.cpp]
//////////////////////////////////////////////////

#ifndef BLABBER_APP_H
	#include "BlabberApp.h"
#endif

#ifndef ABOUT_WINDOW_H
	#include "AboutWindow.h"
#endif

#ifndef GENERIC_FUNCTIONS_H
	#include "GenericFunctions.h"
#endif

AboutWindow *AboutWindow::_instance = NULL;

AboutWindow *AboutWindow::Instance() {
	if (_instance == NULL) {
		float main_window_width, main_window_height;

		// determine what the width and height of the window should be
		main_window_width  = 175;
		main_window_height = 300; 
		
		// create window frame position
		BRect frame(GenericFunctions::CenteredFrame(main_window_width, main_window_height));

		// create window singleton
		_instance = new AboutWindow(frame);
	}

	return _instance;
}

AboutWindow::AboutWindow(BRect frame)
	: BWindow(frame, "About Jabber for Haiku", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) {
	// draw credits
	PopulateCredits();

	// add children
	AddChild(_credits);
}

AboutWindow::~AboutWindow() {
	_instance = NULL;
}
	
void AboutWindow::PopulateCredits() {
	// create the thin (plain) and thick (bold) font
	BFont title(be_plain_font);

	title.SetSize(10.0);
			
	// some colors to play with
	rgb_color yellow = {248, 204, 22, 255};
	rgb_color white  = {255, 255, 255, 255};
	rgb_color red    = {255, 0, 0, 255};
	rgb_color blue   = {100, 100, 255, 255};
	
	// some runs to play with
	text_run tr_title       = {0, title, yellow};
	text_run tr_credit      = {0, title, white};
	text_run tr_blue_credit = {0, title, blue};
	text_run tr_red_credit  = {0, title, red};

	// some run array to play with (simple)
	text_run_array tra_title       = {1, {tr_title}}; 
	text_run_array tra_credit      = {1, {tr_credit}}; 
	text_run_array tra_blue_credit = {1, {tr_blue_credit}}; 
	text_run_array tra_red_credit  = {1, {tr_red_credit}}; 

	// create credits
	_credits = new BTextView(Bounds(), NULL, Bounds(), B_FOLLOW_ALL, B_WILL_DRAW);
	_credits->SetStylable(true);
	_credits->MakeEditable(false);
	_credits->MakeSelectable(false);
	_credits->SetAlignment(B_ALIGN_CENTER);
	_credits->SetWordWrap(true);
	
	// apply a background layer
	_credits->SetViewColor(0, 0, 0, 255);	
	
	// title credit
	_credits->Insert("\n\n", &tra_title);
	
	_credits->Insert("Jabber for Haiku " APP_VERSION "\n\n\n", &tra_title);
	
	_credits->Insert("Freely based on:\n", &tra_title);
	_credits->Insert("Jabber for ", &tra_credit);
	_credits->Insert("B", &tra_blue_credit);
	_credits->Insert("e", &tra_red_credit);
	_credits->Insert("OS\n", &tra_credit);
	_credits->Insert("by\n", &tra_title);
	_credits->Insert("John Blanco\n\n", &tra_credit);

	_credits->Insert("Haiku port:\n", &tra_title);
	_credits->Insert("Frank Paul Silye\n", &tra_credit);
	_credits->Insert("Andrea Anzani\n\n", &tra_credit);

	_credits->Insert("Graphics:\n", &tra_title);
	_credits->Insert("Daniel Fischer\n", &tra_credit);
	_credits->Insert("John Blanco\n", &tra_credit);
	_credits->Insert("zuMi\n", &tra_credit);
	_credits->Insert("Jabber.org\n\n", &tra_credit);
	
	_credits->Insert("Source Code hosted by:\n", &tra_title);
	_credits->Insert("www.osdrawer.net\n", &tra_credit);

/*	_credits->Insert("The Beta Team:\n", &tra_title);
	_credits->Insert("Alan \"void\" Ellis\n", &tra_credit);
	_credits->Insert("Ben Raymond\n", &tra_credit);
	_credits->Insert("Todd Thomas\n", &tra_credit);
	_credits->Insert("Donovan \"Deej\" Schulteis\n", &tra_credit);
	_credits->Insert("Bill Sinclair\n", &tra_credit);
	_credits->Insert("Nutcase\n", &tra_credit);
	_credits->Insert("Bruno \"BGA\" Albuquerque\n", &tra_credit);
	_credits->Insert("Robin \"Serpentor\" Kimzey\n", &tra_credit);
	_credits->Insert("Eric Murphy\n\n", &tra_credit);
	
	_credits->Insert("...And Special Yogi hugs to:\n", &tra_title);
	_credits->Insert("Michele Blanco\n", &tra_credit);
*/		
	// credits
}

