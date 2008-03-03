//////////////////////////////////////////////////
// Blabber [Agent.h]
//     Agent information.
//////////////////////////////////////////////////

#ifndef AGENT_H
#define AGENT_H

#ifndef __STRING__
	#include <string>
#endif

class Agent {
	public:
		        Agent();
		       ~Agent();
		
		string  JID() const;
		string  Name();
		string  Description();
		string  Service() const;
		bool    IsRegisterable() const;
		bool    IsRegistered() const;
		string  Transport();
		bool    IsSearchable();

		string  Username() const;
		string  Password() const;

		void    SetJID(string jid);
		void    SetName(string name);
		void    SetDescription(string description);
		void    SetService(string service);
		void    SetRegisterableFlag(bool is_registerable);
		void    SetRegisteredFlag(bool is_registered);
		void    SetTransport(string transport);
		void    SetSearchableFlag(bool is_searchable);
		
		void    SetUsername(string username);
		void    SetPassword(string password);
		
		void    Register();
		void    UnRegister();
		
	private:
		string _jid;
		string _name;
		string _description;
		string _service;
		bool   _is_registerable;
		string _transport;
		bool   _is_searchable;

		string _username;
		string _password;

		bool   _is_registered;
};

#endif