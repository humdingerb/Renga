//////////////////////////////////////////////////
// Networking [PortTalker.h]
//     A network connection wrapper.
//////////////////////////////////////////////////

#ifndef PORT_TALKER_H
#define PORT_TALKER_H

#ifndef __MAP__
	#include <map>
#endif

#ifndef __STRING__
	#include <string>
#endif

#ifndef _OS_H
	#include <kernel/OS.h>
#endif

#ifndef _LOOPER_H
	#include <Looper.h>
#endif

#ifndef _MESSAGE_H
	#include <Message.h>
#endif

#ifndef _MESSENGER_H
	#include <Messenger.h>
#endif

#ifndef _NETDB_H
	#ifdef BONE
		#include <bone/netdb.h>
	#else
		#include <netdb.h>
	#endif	
#endif

#ifndef _SOCKET_H
	#ifdef BONE
//		#include <bone/oldsocket.h>
		#include <bone/sys/socket.h>
	#else
		#include <sys/socket.h>
	#endif	
#endif

#ifndef KEEP_ALIVE_H
	#include "KeepAlive.h"
#endif

class KeepAlive;

class PortTalker {
public:
	enum    messages   {DATA = 0x68374920};
	enum    netstatus  {RESOLVING_HOSTNAME = 0x68374921, COULD_NOT_RESOLVE_HOSTNAME, CONNECTING_TO_HOST, COULD_NOT_CONNECT};

public:
	 static PortTalker *Instance();

public:
	 virtual           ~PortTalker();

   	 bool               Connect(const char *hostname, const int port, bool keep_alive);
	 bool               IsConnected() const;

	 void               Disconnect();
	 void               OnDisconnect(void (*on_disconnect)(void *), void *argument = NULL);

	 int                Send(const char *data, int len = -1);
	 int                SendFiltered(string data, int len = -1);

   	 void               SetTargetLooper(BLooper *looper);
   	 BLooper           *GetTargetLooper() const;
		
protected:
                        PortTalker();
				
private:
	static PortTalker *_instance;

private:
	void               _Reset(bool hard_disconnect = true);
    static int32       _SpawnListenerThread(void *obj);
    void               _ListenerThread();

private:
	static const int   _MAX_BUFFER_SIZE = 1000;

private:
	int                _socket_token;
	BLooper           *_message_looper;
	thread_id          _listener_thread_id;

	KeepAlive         *_keep_alive;
	void             (*_on_disconnect)(void *);
	void              *_on_disconnect_argument;
};

#endif
