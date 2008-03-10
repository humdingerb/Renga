#include "JabberSocketPlug.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define LOG(X) printf X;

JabberSocketPlug::JabberSocketPlug(){
	
	fReceiverThread = -1;
	fSocket = -1;
}

JabberSocketPlug::~JabberSocketPlug(){
}

int32
JabberSocketPlug::StartConnection(BString fHost, int32 fPort,void* cookie){
	
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
   
	LOG(("DONE: StartConnection to %s:%ld\n",fHost.String(),fPort));
	return 0;	
}


int32
JabberSocketPlug::ReceiveData(BMessage* mdata){
	
	char data[1024];
	int length = 0;
	
	
	while (true) 
	{
		if ((length = (int)recv(fSocket, data, 1023, 0)) > 0) 
		{
			data[length] = 0;
			mdata->AddString("data", data);
			
			#ifdef STDOUT
				printf("\n<< %s\n", data);
			#endif
		} 
			
		ReceivedData(data,length);
	}

	mdata->AddInt32("length", length);
	return length;
}

void
JabberSocketPlug::ReceivedData(const char* data,int32 len)
{
	
}

int
JabberSocketPlug::Send(const BString & xml){
	
	if (fSocket) 
	{
		#ifdef STDOUT
			printf("\n>> %s\n", xml.String());
		#endif
		
		if(send(fSocket, xml.String(), xml.Length(), 0) == -1)
			return -1;
	} 
	
	else
	
	{
		printf("Socket not initialized\n");
		return -1;
	}
	return 0;
}

int		
JabberSocketPlug::StopConnection()
{
	
	//Thread Killing!
	suspend_thread(fReceiverThread);
	
	if(fReceiverThread)	kill_thread(fReceiverThread);
	
	fReceiverThread = 0;
	
	close(fSocket);
	return 0;
}


//--

