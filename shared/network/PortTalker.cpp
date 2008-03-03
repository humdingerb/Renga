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

#ifdef DEBUG
	#ifndef __IOSTREAM__
		#include <iostream>
	#endif
#endif

PortTalker *PortTalker::_instance = NULL;

PortTalker *PortTalker::Instance() {
	if (_instance == NULL) {
		_instance = new PortTalker();
	}
	
	return _instance;
}

PortTalker::PortTalker() {
	// only initialized in beginning, not part of reset
	_socket_token           = -1;
	_listener_thread_id     = -1;
	_on_disconnect          = NULL;
	_on_disconnect_argument = NULL;
	_message_looper         = NULL;
	_keep_alive             = NULL;
	
	// initialize all other values
	_Reset(false);
}

PortTalker::~PortTalker() {
	_instance = NULL;
}

bool PortTalker::Connect(const char *hostname, const int port, bool keep_alive) {
	// connect to socket
	_socket_token = socket(AF_INET, SOCK_STREAM, 0);

	// connect to remote machine
	if (_socket_token >= 0) {
		struct sockaddr_in dest_addr;

		// use DNS to get a host IP
		if (_message_looper) { 
			BMessage msg(RESOLVING_HOSTNAME);
				msg.AddString("hostname", hostname);

			_message_looper->PostMessage(&msg);
		}

		hostent *host = gethostbyname(hostname);

		if (!host) {
			if (_message_looper) {
				BMessage msg(COULD_NOT_RESOLVE_HOSTNAME);
					msg.AddString("hostname", hostname);

				_message_looper->PostMessage(&msg);
			}

			_Reset(false);
			return false;
		}
		
		dest_addr.sin_family      = AF_INET;
		dest_addr.sin_port        = htons(port);
		dest_addr.sin_addr.s_addr = *(ulong *)host->h_addr;

		memset(dest_addr.sin_zero, 0, sizeof(dest_addr.sin_zero)); 

		if (_message_looper) {
			BMessage msg(CONNECTING_TO_HOST);
				msg.AddString("hostname", hostname);
			
			_message_looper->PostMessage(&msg);
		}

		if(connect(_socket_token, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1) {
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
	 return (_socket_token >= 0 && _listener_thread_id > B_NO_ERROR);
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
		bytes_sent = send(_socket_token, data, (len - total_bytes_sent), 0);

		// Accumulate total
		total_bytes_sent += bytes_sent;
	}

	return total_bytes_sent;
}

int PortTalker::SendFiltered(string data, int len) {
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
	if (_socket_token >= 0) {
		#ifdef BONE
			shutdown(_socket_token, SHUTDOWN_BOTH);
		#else
			close(_socket_token);
		#endif

		_socket_token = -1;
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
	char buffer[PortTalker::_MAX_BUFFER_SIZE];

	while(true) {
		// Listen to socket
		ssize_t bytes_received = recv(_socket_token, buffer, PortTalker::_MAX_BUFFER_SIZE - 1, 0);

		// connection dropped BUGBUG make sure 0 return is handled correctly
		if (bytes_received <= 0) {
			// ignore this particular error
			if (strcasecmp(strerror(errno), "Interrupted system call")) {
				_listener_thread_id = -1;
				_Reset(true);
			
				return;
			}
		}
			
		if (bytes_received > 0) {
			// Give the return buffer a NULL
			buffer[bytes_received] = '\0';

			// display results for debuggers
			#if DEBUG
				cout << "[PortTalker Receives " << bytes_received << " bytes]" << buffer << "[Done]" << endl << flush;
			#endif

			if (_message_looper) {
				BMessage msg(PortTalker::DATA);

				msg.AddString("data", buffer);
				msg.AddInt32("length", bytes_received);

				_message_looper->PostMessage(&msg);
			}
		}
		
		// in case networking goes schitz (can't wait for BONE!)
		snooze(50000);
	}

	_listener_thread_id = -1;
}
