#ifndef _config_h_
#define _config_h_

#include <string>
#include "tinyxml.h"

class Config
{
public:
	Config();
	~Config();

public:
	int SetConfig(const char* key, const char* value);
	bool GetConfig(const char* key, std::string& value);
	std::string GetConfig2(const char* key, const char* defaultValue);

	int SetConfig(const char* key, int value);
	bool GetConfig(const char* key, int& value);
	int GetConfig2(const char* key, int defaultValue);
	
	int SetConfig(const char* key, bool value);
	bool GetConfig(const char* key, bool& value);
	bool GetConfig2(const char* key, bool defaultValue);

private:
	TiXmlElement* GetFirstXmlNode(const char* xpath);

private:
	TiXmlDocument m_xml;
};

extern Config g_config;

#endif /* !_config_h_ */
