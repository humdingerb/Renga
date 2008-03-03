//////////////////////////////////////////////////
// Configuration [Configuration.cpp]
//////////////////////////////////////////////////

#include <cstdio>
#include <fstream>

#include "Configuration.h"

Configuration::Configuration(string filename) {
	_filename = filename;
	
	// parse the file
	Reset();
}

Configuration::~Configuration() {
}

const char *Configuration::CheckStatus() {
	return (_parse_error.size() > 0) ? _parse_error.c_str() : "";
}

void Configuration::Reset() {
	char raw_data[255];

	// clear out key/value pairs
	_key_map.clear();

	// open file, check for error	
	ifstream config_stream(_filename.c_str());

	if (!config_stream) {
		_parse_error = "Could not open file '" + _filename + "'";
		return;
	}

	// read a line, parse it, and if there aren't any problems, map it
	while (config_stream.getline(raw_data, 255)) {
		// parse out it out
		pair<string, string> *key_value = _ParseKeyValuePair(string(raw_data));

		// if successful, add to map
		if (key_value) {
			_key_map[key_value->first] = key_value->second;
		}

		// deallocate
		delete key_value;
	}

	// close file	
	config_stream.close();
}

string Configuration::Get(string key) {
	if (_key_map.count(key) > 0) {
		return _key_map[key];
	} else {
		return "";
	}
}
	
pair<string, string> *Configuration::_ParseKeyValuePair(string raw_line) {
	string::size_type key_pos   = string::npos,
	                  equal_pos = string::npos,
	                  value_pos = string::npos;

	// parse for key, = and value
	equal_pos = raw_line.find("=");

	if (equal_pos != string::npos) {
		if (equal_pos > 0) {
			key_pos = 0;
		}

		if (raw_line.size() > (equal_pos + 1)) {
			value_pos = equal_pos + 1;
		}
	}
		
	if (key_pos != string::npos && equal_pos != string::npos && value_pos != string::npos) {
		string key, value;

		// parse out data
		key   = raw_line.substr(key_pos, (equal_pos - key_pos));
		value = raw_line.substr((equal_pos + 1), (raw_line.size() - equal_pos - 1));

		// prune out extra whitespace
		key   = _PruneWhitespace(key);
		value = _PruneWhitespace(value);

		// validate
		if (key.size() > 0 && value.size() > 0) {
			return new pair<string, string>(key, value);
		} 
	}
	
	return NULL;
}

string Configuration::_PruneWhitespace(string text) {
	// prune opening whitespace
	while(text.size() > 0 && isspace(text[0])) {
		text.erase(0, 1);
	}

	// prune trailing whitespace
	while(text.size() > 0 && isspace(text[text.size() - 1])) {
		text.erase(text.size() - 1, 1);
	}

	return text;
}
