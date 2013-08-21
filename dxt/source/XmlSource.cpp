#include "XmlSource.h"

XmlSource::XmlSource()
{
}

XmlSource::~XmlSource()
{
	for(TXmls::iterator it=m_xmls.begin(); it!=m_xmls.end(); ++it)
	{
		TiXmlDocument* doc = it->second;
		delete doc;
	}

	m_xmls.clear();
	m_xpaths.clear();
}

bool XmlSource::Add(const char* xml)
{
	TiXmlDocument* doc = new TiXmlDocument();
	doc->Parse(xml);
	if(doc->Error())
	{
		delete doc;
		return false;
	}

	const char* command = GetCommand(doc);
	m_xmls.insert(std::make_pair(NULL==command?"":command, doc));
	return true;
}

bool XmlSource::Foreach(const char *xpath)
{
	const TiXmlElement* node = GetFirstXmlNode(xpath);
	if(NULL == node)
		return false;

	assert(m_xmls.size() > 0);
	const std::string name = m_xmls.begin()->first;
	m_xpaths.push_front(std::make_pair(name, node));
	return true;
}

bool XmlSource::Foreach(const char* xpath, const char* match)
{
	const TiXmlElement* node = GetFirstXmlNode(xpath, match);
	if(NULL == node)
		return false;

	m_xpaths.push_front(std::make_pair(match, node));
	return true;
}

bool XmlSource::ForeachNext()
{
	if(m_xpaths.size() <= 0)
		return false;

	TXPaths::iterator it = m_xpaths.begin();
	assert(it != m_xpaths.end());
	std::string name = it->first;
	const TiXmlElement* node = it->second;
	assert(node);

	m_xpaths.erase(it);
	node = node->NextSiblingElement(node->Value());	
	if(node)
	{
		m_xpaths.push_front(std::make_pair(name, node));
		return true;
	}
	return false;
}

bool XmlSource::GetName(const char* path, std::string& name) const
{
	const TiXmlElement* node = GetFirstXmlNode(path);
	if(NULL == node)
		return false;

	const char* p = node->Value();
	if(NULL == p)
		return false;

	name = p;
	return true;
}

bool XmlSource::GetValue(const char* xpath, std::string& value) const
{
	const TiXmlElement* node = GetFirstXmlNode(xpath);
	if(NULL == node)
		return false;

	const char* p = node->GetText();
	value = p?p:"";
	return true;
}

bool XmlSource::GetValue(const char* xpath, const char* match, std::string& value) const
{
	const TiXmlElement* node = GetFirstXmlNode(xpath, match);
	if(NULL == node)
		return false;

	const char* p = node->GetText();
	value = p?p:"";
	return true;
}

bool XmlSource::GetAttr(const char* path, const char* name, std::string& value) const
{
	const TiXmlElement* node = GetFirstXmlNode(path);
	if(NULL == node)
		return false;

	const char* p = node->Attribute(name);
	if(NULL == p)
		return false;

	value = p;
	return true;
}

static const TiXmlNode* GetFirstElement(const TiXmlNode* node, const char* xpath)
{
	if(NULL == node)
		return NULL;

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
		return GetFirstElement(node, xpath+1);
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
			const TiXmlElement* child = node->FirstChildElement(name);
			if(NULL == child)
				return NULL;
			return GetFirstElement(child, p+1);
		}
	}
}

const TiXmlElement* XmlSource::GetFirstXmlNode(const char* xpath) const
{
	if(NULL == xpath)
		return NULL;

	if(0 == strncmp(xpath, "//", 2))
	{
		// un-implement
		return NULL;
	}
	else if('/' == *xpath)
	{
		if(m_xmls.size() == 0)
			return NULL;

		// try xml root element
		TiXmlDocument* doc = m_xmls.begin()->second;
		TiXmlElement* root = doc->RootElement();
		return (const TiXmlElement*)GetFirstElement(root, xpath+1);
	}
	else
	{
		return (const TiXmlElement*)GetFirstElement(GetContextXmlNode(), xpath);
	}
}

const TiXmlElement* XmlSource::GetFirstXmlNode(const char* xpath, const char* match) const
{
	if(NULL == xpath)
		return NULL;

	if(0 == strncmp(xpath, "//", 2))
	{
		// un-implement
		return NULL;
	}
	else if('/' == *xpath)
	{
		TXmls::const_iterator it2 = m_xmls.find(match);
		if(it2 == m_xmls.end())
			return NULL;

		const TiXmlDocument* doc = it2->second;
		const TiXmlElement* root = doc->RootElement();
		return (const TiXmlElement*)GetFirstElement(root, xpath+1);
	}
	else
	{
		return (const TiXmlElement*)GetFirstElement(GetContextXmlNode(match), xpath);
	}
}
