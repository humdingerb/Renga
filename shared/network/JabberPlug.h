/*
	Interface for a highlevel connection class
	- basic implementations:
		JabberSSLPlug	 (new code used by GoogleTalk)
		JabberSocketPLug (old code BONE/net_server)

	22 sept. 2005 by Andrea Anzani (andrea.anzani@gmail.com)
*/
#ifndef JabberPlug_H_
#define JabberPlug_H_

#include <String.h>

class JabberPlug {

	public:
		
		virtual 			~JabberPlug(){};
		virtual	   int32	StartConnection(BString fHost, int32 fPort,void*) = 0;
		virtual	   int		Send(const BString & xml) = 0;	
		virtual	   int		StopConnection() = 0;
		virtual	   bool		IsConnected() = 0;
		
		enum    messages   {DATA = 0x68374920};
		enum    netstatus  {RESOLVING_HOSTNAME = 0x68374921, COULD_NOT_RESOLVE_HOSTNAME, CONNECTING_TO_HOST, COULD_NOT_CONNECT};

};

#endif
//.
