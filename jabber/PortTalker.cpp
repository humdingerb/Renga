//////////////////////////////////////////////////
// Networking [PortTalker.cpp]
//////////////////////////////////////////////////

#ifndef PORT_TALKER_H
	#include "PortTalker.h"
#endif

#ifndef __CSTRING__
	#include <cstring>
#endif

#ifndef _ERRNO_H_
	#include <errno.h>
#endif

#ifndef __STRING__
	#include <string>
#endif

#ifndef __IOSTREAM__
	#include <iostream>
#endif

using namespace std;

#include "JabberSSLPlug.h"
#include "JabberSocketPlug.h"

PortTalker *PortTalker::_instance = NULL;

PortTalker *PortTalker::Instance() {
	if (_instance == NULL) {
		_instance = new PortTalker();
	}

	return _instance;
}

PortTalker::PortTalker() {
	// only initialized in beginning, not part of reset
	_listener_thread_id     = -1;
	_on_disconnect          = NULL;
	_on_disconnect_argument = NULL;
	_message_looper         = NULL;
	_keep_alive             = NULL;
	_plug					= NULL;

	// initialize all other values
	_Reset(false);
}

PortTalker::~PortTalker() {
	_instance = NULL;
}

bool PortTalker::Connect(const char *hostname, const int port, __attribute__((unused)) bool keep_alive, bool useSSL) {
	// connect to socket

	//safety: if there is already a _plug defined, we delete it first.
	if(_plug)
		delete _plug;

	if(useSSL)
		_plug	= new JabberSSLPlug();
	else
		_plug	= new JabberSocketPlug();

	int32 result= _plug->StartConnection(BString(hostname), port);
	// connect to remote machine
	if (result >= 0) {

		if(result != 0) {
			if (_message_looper) {
				BMessage msg(result);
				msg.AddString("hostname", hostname);
				_message_looper->PostMessage(&msg);
			}
			_Reset(false);
			return false;
		}
	} else {
		_Reset(false);
		return false;
	}

	// spawn listener thread (communication from remote machine)
	resume_thread(_listener_thread_id = spawn_thread(PortTalker::_SpawnListenerThread, "port_listener", B_LOW_PRIORITY, this));

	// we have a successful connection
	return true;
}

bool PortTalker::IsConnected() const {
	 return (_plug && _plug->IsConnected() && _listener_thread_id > B_NO_ERROR);
}

void PortTalker::Disconnect() {
	_Reset(false);
}

void PortTalker::OnDisconnect(void (*on_disconnect)(void *), void *argument) {
	_on_disconnect = on_disconnect;
	_on_disconnect_argument = argument;
}

int PortTalker::Send(const char *data, int len) {
	int bytes_sent       = 0;
	int total_bytes_sent = 0;

	// calculate length?
	if (len == -1) {
		len = strlen(data);
	}

	#ifdef DEBUG
		cout << "[PortTalker Sends (" << len << ")]" << data << "[Done]" << endl << flush;
	#endif

	// don't send if we're not connected
	if (!IsConnected()) {
 		return 0;
	}

	// Send data and return number of bytes sent
	for (int i=0; (total_bytes_sent < len) && (i < 5); ++i) {
		// Advance pointer as bytes are sent
		data += bytes_sent;

		if (!IsConnected()) {
	 		return total_bytes_sent;
		}

		// Send the data
		BString sdata(data,(len - total_bytes_sent));
		bytes_sent = _plug->Send(sdata);

		// Accumulate total
		total_bytes_sent += bytes_sent;
	}

	return total_bytes_sent;
}

int PortTalker::SendFiltered(std::string data, int len) {
	// remove control characters and unprintables
	for (unsigned int i = 0; i < data.size(); ++i) {
		if (data[i] == 27) {
			data.erase(i, 1);
			--i;
		}
	}

	// send filtered string
	return Send(data.c_str(), len);
}

void PortTalker::SetTargetLooper(BLooper *looper) {
	_message_looper = looper;
}

BLooper *PortTalker::GetTargetLooper() const {
	return _message_looper;
}

void PortTalker::_Reset(bool hard_disconnect) {
	// if we're connected
	if (_listener_thread_id > B_NO_ERROR) {
			kill_thread(_listener_thread_id);

			_listener_thread_id = -1;
	}

	// end any running processes
	if (_plug )
	{
		delete _plug;
		_plug = NULL;
	}

	// callback
	if (hard_disconnect && _on_disconnect) {
		(*_on_disconnect)(_on_disconnect_argument);
	}
}

int32 PortTalker::_SpawnListenerThread(void *obj) {
	((PortTalker *)obj)->_ListenerThread();

	// Don't care about the return value
	return 1;
}

void PortTalker::_ListenerThread() {
	//char buffer[PortTalker::_MAX_BUFFER_SIZE];

	BMessage msg(PortTalker::DATA);

	while(true) {
		// Listen to socket
		msg.MakeEmpty();
		ssize_t bytes_received = ((JabberPlug*)_plug)->ReceiveData(&msg);

		// connection dropped BUGBUG make sure 0 return is handled correctly
		if (bytes_received <= 0) {
			// ignore this particular error
			if (strcasecmp(strerror(errno), "Interrupted system call")) {
				_listener_thread_id = -1;
				_Reset(true);
				return;
			}
		}

		if (bytes_received > 0 && _message_looper)
		{
			_message_looper->PostMessage(&msg);
		}

		// in case networking goes schitz (can't wait for BONE!)
		snooze(50000);
	}

	_listener_thread_id = -1;
}
