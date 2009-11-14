#ifndef KEEP_ALIVE_H
#define KEEP_ALIVE_H

#include "PortTalker.h"

class PortTalker;

class KeepAlive {
public:
	                   KeepAlive(PortTalker *port_talker);
	                  ~KeepAlive();
private:
    static int32      _SpawnKeepAliveThread(void *);
    void              _KeepAliveThread();

	PortTalker       *_port_talker;
	thread_id         _keep_alive_thread_id;
};

#endif