//////////////////////////////////////////////////
// Generic [ProxyLooper.h]
//     Serves as a "bodyguard" to a view, fielding
//     messages and serving them to the view.
//////////////////////////////////////////////////

#ifndef PROXY_LOOPER_H
#define PROXY_LOOPER_H

#ifndef _LOOPER_H
	#include <app/Looper.h>
#endif

#ifndef _VIEW_H
	#include <interface/View.h>
#endif

class ProxyLooper : public BLooper {
public:
	        ProxyLooper(BView *target_view);
	       ~ProxyLooper();
	       
	void    MessageReceived(BMessage *msg);	     
		
private:
	BView *_target_view;
};

#endif