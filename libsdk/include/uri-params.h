#ifndef _uri_params_h_
#define _uri_params_h_

#include "cstringext.h"
#include "cppstringext.h"
#include "url.h"
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>

class URIParams
{
public:
	URIParams(void* uri=NULL){ if(uri) Init(uri); };

public:
	const std::string& Get(const std::string& key) const
	{
		TPairs::const_iterator it = m_params.find(key);
		if(it == m_params.end())
			return sc_emptystring;
		return it->second;
	}

	bool Get(const std::string& key, std::string& v) const
	{
		TPairs::const_iterator it = m_params.find(key);
		if(it == m_params.end())
			return false;
		v = it->second;
		return true;
	}

	bool Get(const std::string& key, int& v) const
	{
		TPairs::const_iterator it = m_params.find(key);
		if(it == m_params.end())
			return false;
		v = atoi(it->second.c_str());
		return true;
	}

	bool Get(const std::string& key, bool& v) const
	{
		TPairs::const_iterator it = m_params.find(key);
		if(it == m_params.end())
			return false;
		v = 0==stricmp(it->second.c_str(), "true");
		return true;
	}

	int Get2(const std::string& key, int defaultValue) const
	{
		TPairs::const_iterator it = m_params.find(key);
		if(it == m_params.end())
			return defaultValue;
		int v = atoi(it->second.c_str());
		return v;
	}

	bool Get2(const std::string& key, bool defaultValue) const
	{
		TPairs::const_iterator it = m_params.find(key);
		if(it == m_params.end())
			return defaultValue;
		bool v = 0==stricmp(it->second.c_str(), "true");
		return v;
	}

public:
	void Init(void* uri)
	{
		m_params.clear();
		for(int i=0; i<url_getparam_count(uri); i++)
		{
			const char *name, *value;
			if(0 != url_getparam(uri, i, &name, &value))
				continue;
			m_params.insert(std::make_pair(std::string(name), std::string(value)));
		}
	}

private:
	typedef std::map<std::string, std::string> TPairs;
	TPairs m_params;
};

#endif /* !_uri_params_h_ */
