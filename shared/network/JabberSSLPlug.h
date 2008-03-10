/*
	JabberSSLPlug (written for GoogleTalk compatibility)
	22 sept. 2005 by Andrea Anzani (andrea.anzani@gmail.com)
*/

#ifndef JabberSSLPlug_H_
#define JabberSSLPlug_H_

#include "JabberPlug.h"

#include <OS.h>
#include <Message.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>


// public JabberPlug
class JabberSSLPlug : public JabberPlug {

	public:
			JabberSSLPlug(BString forceserver = NULL,int32 port = 0);
		   ~JabberSSLPlug();
	//private:
		
			   int32	StartConnection(BString fHost, int32 fPort,void* cook);//if >= 0 it's ok.
			   int32 	ReceiveData(BMessage *);    	//thread called function
			   int		Send(const BString & xml);	//if >= 0 it's ok.		
			   int		StopConnection();	
	
			   void		ReceivedData(const char* data,int32);
			   

			   bool		IsConnected() { if(bio==NULL || ctx==NULL) return false; else return true;}
	private:
	
	
		BIO* 		bio;
    	SSL_CTX* 	ctx; 
    	
    	BString	ffServer;
    	int32	ffPort;
		
						
};

#endif

//--
