#include "config.h"
#include "cstringext.h"
#include "StrConvert.h"

Config g_config;

Config::Config()
{
	m_xml.LoadFile("config.xml");
}

Config::~Config()
{
}

int Config::SetConfig(const char *key, const char *value)
{
	if(NULL == value)
		return -1;

	TiXmlElement* node = GetFirstXmlNode(key);
	if(NULL == node)
		return -1;

	node->Clear();
	TiXmlText* child = new TiXmlText(value);
	node->LinkEndChild(child);
	m_xml.SaveFile();
	return 0;
}

bool Config::GetConfig(const char* key, std::string& value)
{
	TiXmlElement* root = m_xml.RootElement();
	if(NULL == root)
		return false;

	TiXmlElement* node = GetFirstXmlNode(key);
	if(NULL == node)
		return false;

	const char* v = node->GetText();
	if(NULL == v)
		return false;

	value = v;
	return true;
}

std::string Config::GetConfig2(const char* key, const char* defaultValue)
{
	TiXmlElement* root = m_xml.RootElement();
	if(NULL == root)
		return defaultValue;

	TiXmlElement* node = GetFirstXmlNode(key);
	if(NULL == node)
		return defaultValue;

	const char* v = node->GetText();
	if(NULL == v)
		return defaultValue;

	return v;
}

int Config::SetConfig(const char* key, int value)
{
	SetConfig(key, ToString(value));
	return 0;
}

bool Config::GetConfig(const char *key, int &value)
{
	std::string v;
	if(!GetConfig(key, v))
		return false;

	value = atoi(v.c_str());
	return true;
}

int Config::GetConfig2(const char *key, int defaultValue)
{
	std::string v = GetConfig2(key, (const char*)ToString(defaultValue));
	return atoi(v.c_str());
}

int Config::SetConfig(const char* key, bool value)
{
	SetConfig(key, ToString(value));
	return 0;
}

bool Config::GetConfig(const char* key, bool& value)
{
	std::string v;
	if(!GetConfig(key, v))
		return false;

	value = 0==stricmp(v.c_str(), "true");
	return true;
}

bool Config::GetConfig2(const char* key, bool defaultValue)
{
	std::string v = GetConfig2(key, (const char*)ToString(defaultValue));
	return 0==stricmp(v.c_str(), "true");
}

static TiXmlNode* GetFirstElement(TiXmlNode* node, const char* xpath)
{
	if(0 == strncmp("..", xpath, 2))
	{
		xpath += 2;
		if(0 == *xpath)
			return node->Parent();

		if('/' != *xpath)
			return NULL;
		return GetFirstElement(node->Parent(), xpath+1);
	}
	else if(0 == strncmp(".", xpath, 1))
	{
		xpath += 1;
		if(0 == *xpath)
			return node;

		if('/' != *xpath)
			return NULL;
		return GetFirstElement(node->Parent(), xpath+1);
	}
	else
	{
		const char* p = strchr(xpath, '/');
		if(NULL == p)
		{
			return node->FirstChildElement(xpath);
		}
		else
		{
			std::string name(xpath, p-xpath);
			TiXmlElement* child = node->FirstChildElement(name);
			if(NULL == child)
				return NULL;
			return GetFirstElement(child, p+1);
		}
	}
}

TiXmlElement* Config::GetFirstXmlNode(const char* xpath)
{
	if(NULL == xpath)
		return NULL;

	if('/' == *xpath)
		xpath += 1;
	
	TiXmlElement* root = m_xml.RootElement();
	return (TiXmlElement*)GetFirstElement(root, xpath);
}
