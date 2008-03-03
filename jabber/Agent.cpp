//////////////////////////////////////////////////
// Blabber [Agent.cpp]
//////////////////////////////////////////////////

#ifndef AGENT_H
	#include "Agent.h"
#endif

Agent::Agent() {
	SetJID("");
	SetName("");
	SetDescription("");
	SetService("");
	SetRegisterableFlag(false);
	SetRegisteredFlag(false);
	SetTransport("");
	SetSearchableFlag(false);
}

Agent::~Agent() {
}
		
string Agent::JID() const {
	return _jid;
}

string Agent::Name() {
	return _name;
}

string Agent::Description() {
	return _description;
}

string Agent::Service() const {
	return _service;
}

bool Agent::IsRegisterable() const {
	return _is_registerable;
}

bool Agent::IsRegistered() const {
	return _is_registered;
}

string Agent::Transport() {
	return _transport;
}

bool Agent::IsSearchable() {
	return _is_searchable;
}

string Agent::Username() const {
	return _username;
}

string Agent::Password() const {
	return _password;
}

void Agent::SetJID(string jid) {
	_jid = jid;
}

void Agent::SetName(string name) {
	_name = name;
}

void Agent::SetDescription(string description) {
	_description = description;
}

void Agent::SetService(string service) {
	_service = service;
}

void Agent::SetRegisterableFlag(bool is_registerable) {
	_is_registerable = is_registerable;
}

void Agent::SetRegisteredFlag(bool is_registered) {
	if (_is_registered != is_registered) {
		_is_registered = is_registered;
	
		
		if (_is_registered == false) {
			SetUsername("");
			SetPassword("");
		}
	}
}

void Agent::SetTransport(string transport) {
	_transport = transport;
}

void Agent::SetSearchableFlag(bool is_searchable) {
	_is_searchable = is_searchable;
}

void Agent::SetUsername(string username) {
	_username = username;
}

void Agent::SetPassword(string password) {
	_password = password;
}

void Agent::Register() {
	_is_registered = true;
}

void Agent::UnRegister() {
	_is_registered = false;
}