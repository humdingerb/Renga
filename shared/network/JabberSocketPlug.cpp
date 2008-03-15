#include "JabberSocketPlug.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define LOG(X) printf X;
//#define STDOUT

JabberSocketPlug::JabberSocketPlug()
{
	fSocket = -1;
}

JabberSocketPlug::~JabberSocketPlug()
{
	close(fSocket);
}

int32
JabberSocketPlug::StartConnection(BString fHost, int32 fPort){
	
	LOG(("StartConnection to %s:%ld\n",fHost.String(),fPort));
	
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
				
    if (inet_aton(fHost.String(), &remoteAddr.sin_addr) == 0)
	{
       	struct hostent * remoteInet(gethostbyname(fHost.String()));
       	if (remoteInet)
       		remoteAddr.sin_addr = *((in_addr *)remoteInet->h_addr_list[0]);
       	else 
       	{
			printf("failed (remoteInet) [%s]\n",fHost.String());
			return CONNECTING_TO_HOST;
		}
    }

    remoteAddr.sin_port = htons(fPort);
    
    if ((fSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
    	printf("failed to create socket\n");
    	fSocket = -1;
    	return CONNECTING_TO_HOST;
    }
    
    if (connect(fSocket, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0) 
    {
	   	printf("failed to connect socket\n");
    	fSocket = -1;
    	return CONNECTING_TO_HOST;
    }
   
	LOG(("DONE: StartConnection to %s:%ld -- fSocket %ld\n", fHost.String(), fPort, fSocket));
	return 0;	
}


int32
JabberSocketPlug::ReceiveData(BMessage* mdata){
	
	char data[1024];
	int length = 0;
	
	if ((length = (int)recv(fSocket, data, 1023, 0)) > 0) 
	{
		data[length] = 0;
		mdata->AddString("data", data);
			
		#ifdef STDOUT
			printf("\n<< %s\n", data);
		#endif
	}

	mdata->AddInt32("length", length);
	return length;
}


int
JabberSocketPlug::Send(const BString & xml)
{
	return send(fSocket, xml.String(), xml.Length(), 0);
}


//--

