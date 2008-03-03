//////////////////////////////////////////////////
// Configuration [Configuration.h]
//////////////////////////////////////////////////

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <map>
#include <string>

class Configuration {
public:
	                       Configuration(string filename);
	                      ~Configuration();
	const char            *Configuration::CheckStatus();
	
	void                   Reset();

	string                 Get(string key);
	
private:
	pair<string, string> *_ParseKeyValuePair(string raw_line);
	string                _PruneWhitespace(string text);
	
	string                _filename;
	string                _parse_error;
	map<string, string>   _key_map;
};

#endif
