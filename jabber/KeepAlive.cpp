#include "KeepAlive.h"

KeepAlive::KeepAlive(PortTalker *port_talker) {
	_keep_alive_thread_id = -1;
	_port_talker          = port_talker;

	// Spawn listener thread (communication from remote machine)
	resume_thread(_keep_alive_thread_id = spawn_thread(KeepAlive::_SpawnKeepAliveThread, "keep_alive", B_NORMAL_PRIORITY, this));
}

KeepAlive::~KeepAlive() {
	if (_keep_alive_thread_id > 0)
		kill_thread(_keep_alive_thread_id);
}

int32 KeepAlive::_SpawnKeepAliveThread(void *obj) {
	((KeepAlive *)obj)->_KeepAliveThread();
	
	// Don't care about the return value
	return 1;
}

void KeepAlive::_KeepAliveThread() {
	while(1) {
		if (_port_talker->IsConnected()) {
			_port_talker->Send(" ");
		}
		
		snooze(60000000);
	}
}
