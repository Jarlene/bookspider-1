#include <string>
#include "dom.h"
#include <assert.h>
#include <stdlib.h>
#include "domutil.h"
#include "cppstringext.h"

dom::dom(void* doc)
{
	m_doc = (domdoc_t*)doc;
}

dom::~dom()
{
}

// div		=> (div, 1)
// div[3]	=> (div, 3)
// div[class="style",id="xid"] => (div, 3, ("class", "style"), ("id", "xid"))

typedef std::list<std::pair<std::string, std::string> > TXProps;
static void ParseXPathName(const char* name, std::string& tag, int& idx, TXProps& props)
{
	const char* p = strchr(name, '[');
	if(p)
	{
		if(strchr(p+1, '='))
		{
			std::vector<std::string> items;
			Split(p+1, ',', items);
			for(size_t i=0; i<items.size(); i++)
			{
				std::string first, second;
				if(SplitPair(items[i].c_str(), '=', first, second, " \"\']"))
					props.push_back(std::make_pair(first, second));
			}
		}
		else
		{
			idx = atoi(p+1);
		}
		tag.assign(name, p-name);
	}
	else
	{
		idx = 1;
		tag = name;
	}
}

static bool CheckNodeProp(const domnode_t* node, const TXProps& props)
{
	for(TXProps::const_iterator it=props.begin(); it!=props.end(); ++it)
	{
		const char* value = domnode_attr_find(node, it->first.c_str());
		if(!value || stricmp(value, it->second.c_str()))
			return false;
	}
	return true;
}

static const domnode_t* DOMFindChild(const domnode_t* node, const char* name)
{
	int i = 0;
	int idx = 0;
	TXProps props;
	std::string tag;
	ParseXPathName(name, tag, idx, props);

	const domnode_t* child = node->child;
	while(child)
	{
		// "*" match anything
		if(0==strcmp("*", tag.c_str()))
			return child;

		if(child->name && 0==stricmp(child->name, tag.c_str()))
		{
			if(props.size() > 0)
			{
				if(CheckNodeProp(child, props))
					return child;
			}
			else
			{
				// match div[3]
				if(++i == idx)
					return child;
			}
		}

		child = (child->next==node->child)?NULL:child->next;
	}
	return NULL;
}

static const domnode_t* DOMFindElement(const domnode_t* node, const char* name)
{
	if(0 == strcmp("..", name)) // parent
	{
		return node->parent;
	}
	else if(0 == strcmp(".", name)) // self
	{
		return node;
	}
	else if(0 == strcmp("+", name)) // next sibling
	{
		return node->next;
	}
	else if(0 == strcmp("-", name)) // previous sibling
	{
		return node->prev;
	}
	else
	{
		return DOMFindChild(node, name);
	}
}

static const domnode_t* DOMFindElementByPath(const domnode_t* node, const char* xpath)
{
	assert(node);
	assert(xpath);

	const char* p = strchr(xpath, '/');
	if(NULL == p)
	{
		return DOMFindElement(node, xpath);
	}
	else
	{
		std::string name(xpath, p-xpath);
		const domnode_t* child = DOMFindElement(node, name.c_str());
		if(NULL == child)
			return NULL;
		return DOMFindElementByPath(child, p+1);
	}
}

const domnode_t* dom::FindElement(const char* path) const
{
	if(NULL == path)
		return NULL;

	if(0 == strncmp(path, "//", 2))
	{
		path += 2;
		const char* p = strchr(path, '/');
		if(!p)
			return FindElementById(path);
		
		std::string name(path, p-path);
		const domnode_t* node = FindElementById(name.c_str());
		if(!node)
			return NULL;
		return DOMFindElementByPath(node, p+1);
	}
	else if('/' == *path)
	{
		return DOMFindElementByPath(m_doc->root, path+1);
	}
	else
	{
		return DOMFindElementByPath(GetContextElement(), path);
	}
}

static const domnode_t* DOMFindChildById(const domnode_t* parent, const char* id)
{
	const domnode_t* node = parent->child;
	while(node)
	{
		const char* value = domnode_attr_find(node, "id");
		if(value && 0==stricmp(value, id))
			return node;

		// find children
		const domnode_t* child = DOMFindChildById(node, id);
		if(child)
			return child;

		node = (node->next==parent->child)?NULL:node->next;
	}
	return NULL;
}

const domnode_t* dom::FindElementById(const char* id) const
{
	return DOMFindChildById(m_doc->root, id);
}

const domnode_t* dom::GetContextElement() const
{
	if(m_paths.empty())
		return m_doc->root;
	return m_paths.front();
}

bool dom::Foreach(const char* path)
{
	const domnode_t* node = FindElement(path);
	if(!node)
		return false;

	m_paths.push_front(node);
	return true;
}

bool dom::ForeachNext()
{
	if(m_paths.empty())
		return false;

	const domnode_t* node = m_paths.front();
	m_paths.pop_front();
	if(node->next != node->parent->child)
	{
		m_paths.push_front(node->next);
		return true;
	}
	return false;
}

bool dom::GetName(const char* path, std::string& name) const
{
	const domnode_t* node = FindElement(path);
	if(!node)
		return false;
	return GetName(node, name);
}

bool dom::GetValue(const char* path, std::string& value) const
{
	const domnode_t* node = FindElement(path);
	if(!node)
		return false;
	return GetValue(node, value);
}

bool dom::GetAttr(const char* path, const char* name, std::string& value) const
{
	const domnode_t* node = FindElement(path);
	if(!node)
		return false;
	return GetAttr(node, name, value);
}

bool dom::GetName(const domnode_t* node, std::string& name) const
{
	if(node->name)
		return false;

	name = node->name;
	return true;
}

bool dom::GetValue(const domnode_t* node, std::string& value) const
{
	assert(node);
	if(!node->name)
	{
		value = node->padding?node->padding:"";
		return true;
	}

	const domnode_t* child = node->child;
	if(child && !child->name)
	{
		value = child->padding?child->padding:"";
		return true;
	}
	
	return false;
}

bool dom::GetAttr(const domnode_t* node, const char* name, std::string& value) const
{
	const char* p = domnode_attr_find(node, name);
	if(!p)
		return false;

	value = p;
	return true;
}

bool dom::GetAttr(const domnode_t* node, const char* name, double& value) const
{
	std::string v;
	if(!GetAttr(node, name, v))
		return false;
	value = atof(v.c_str());
	return true;
}

bool dom::GetAttr(const domnode_t* node, const char* name, bool& value) const
{
	std::string v;
	if(!GetAttr(node, name, v))
		return false;
	value = v=="true"||v=="TRUE"||v=="True";
	return true;
}

bool dom::GetAttr(const domnode_t* node, const char* name, int& value) const
{
	std::string v;
	if(!GetAttr(node, name, v))
		return false;
	value = atoi(v.c_str());
	return true;
}
