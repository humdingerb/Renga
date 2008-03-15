/*
	
	JabberSocketPlug (old BONE code from IMKIT project)

	22 sept. 2005 by Andrea Anzani (andrea.anzani@gmail.com
*/
#ifndef JabberSocketPlug_H_
#define JabberSocketPlug_H_

#include "JabberPlug.h"
#include <Locker.h>
#include <Message.h>

class JabberSocketPlug : public JabberPlug {

	public:
					 JabberSocketPlug();
		    virtual ~JabberSocketPlug();
		
			   int32	StartConnection(BString fHost, int32 fPort);//if >= 0 it's ok.
		 	   int32 	ReceiveData(BMessage* data);    	//thread called function
			   int		Send(const BString & xml);	//if >= 0 it's ok.		

	
			   bool		IsConnected() { return (fSocket >= 0); }
	private:
	
	
		int32 						fSocket;
};

#endif

//--
