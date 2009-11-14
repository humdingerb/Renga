/*
	JabberSSLPlug (written for GoogleTalk compatibility)
	22 sept. 2005 by Andrea Anzani (andrea.anzani@gmail.com)
*/

#ifndef JabberSSLPlug_H_
#define JabberSSLPlug_H_

#include "JabberPlug.h"

#include <OS.h>
#include <String.h>
#include <Message.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>


class JabberSSLPlug : public JabberPlug
{


	public:
						JabberSSLPlug();
					   ~JabberSSLPlug();

			   int32	StartConnection(BString fHost, int32 fPort);
			   int32 	ReceiveData(BMessage *);    //thread called function
			   int		Send(const BString & xml);

			   bool		IsConnected() { return !(bio == NULL || ctx == NULL); }
	private:


		BIO* 		bio;
    	SSL_CTX* 	ctx;

    	BString		ffServer;
    	int32		ffPort;

};

#endif

//--
