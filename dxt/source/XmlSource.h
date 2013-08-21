#ifndef _XmlSource_h_
#define _XmlSource_h_

#include <map>
#include <list>
#include <string>
#include "ISource.h"
#include "tinyxml.h"

class XmlSource : public ISource
{
public:
	XmlSource();
	~XmlSource();

	bool Add(const char* xml);

public:
	virtual bool Foreach(const char* path);
	virtual bool Foreach(const char* path, const char* match);
	virtual bool ForeachNext();

	virtual bool GetName(const char* path, std::string& name) const;

	virtual bool GetValue(const char* path, std::string& value) const;
	virtual bool GetValue(const char* path, const char* match, std::string& value) const;

	virtual bool GetAttr(const char* path, const char* name, std::string& value) const;

private:
	const TiXmlElement* GetFirstXmlNode(const char* xpath) const;
	const TiXmlElement* GetFirstXmlNode(const char* xpath, const char* match) const;

	inline const TiXmlElement* GetContextXmlNode() const
	{
		// try current context path
		if(m_xpaths.size() > 0)
		{
			const TiXmlElement* node = m_xpaths.begin()->second;
			assert(node);
			return node;
		}

		if(m_xmls.size() == 0)
			return NULL;

		// try xml root element
		const TiXmlDocument* doc = m_xmls.begin()->second;
		return doc->RootElement();
	}

	inline const TiXmlElement* GetContextXmlNode(const char* match) const
	{
		// find context path
		TXPaths::const_iterator it = FindXPath(match);
		if(it != m_xpaths.end())
		{
			const TiXmlElement* node = it->second;
			assert(node);
			return node;
		}

		// get xml document root node
		TXmls::const_iterator it2 = m_xmls.find(match);
		if(it2 != m_xmls.end())
		{
			const TiXmlDocument* doc = it2->second;
			return doc->RootElement();
		}

		return NULL;
	}

	inline const char* GetCommand(const TiXmlDocument* doc) const
	{
		const TiXmlElement* root = doc->RootElement();
		if(NULL == root)
			return NULL;

		const char* command = root->Attribute("command");
		if(NULL != command)
			return command;
		return root->Attribute("reply");
	}

private:
	typedef std::list<std::pair<std::string, const TiXmlElement*> > TXPaths;

	TXPaths::const_iterator FindXPath(const std::string& xpath) const
	{
		for(TXPaths::const_iterator it=m_xpaths.begin(); it!=m_xpaths.end(); ++it)
		{
			if(0 == strcmp(xpath.c_str(), it->first.c_str()))
				return it;
		}
		return m_xpaths.end();
	}

private:
	typedef std::map<std::string, TiXmlDocument*> TXmls;
	TXmls m_xmls;

	TXPaths m_xpaths;
};

#endif /* !_XmlSource_h_ */
