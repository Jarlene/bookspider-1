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
static void ParseXPathName(const char* name, std::string& label, int& idx, TXProps& props)
{
	idx = -1;
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
				if(SplitPair(items[i].c_str(), '=', first, second, "@ \"\']"))
					props.push_back(std::make_pair(first, second));
			}
		}
		else
		{
			idx = atoi(p+1);
		}
		label.assign(name, p-name);
	}
	else
	{
		label = name;
	}
}

static bool DOMCheckNode(const domnode_t* node, const char* label, const TXProps& attrs)
{
	assert(label);
	if(!node->name || (0!=stricmp(node->name, label) && 0!=strcmp("*", label)))
		return false;

	for(TXProps::const_iterator it=attrs.begin(); it!=attrs.end(); ++it)
	{
		const char* value = domnode_attr_find(node, it->first.c_str());
		if(!value || stricmp(value, it->second.c_str()))
			return false;
	}

	return true;
}

static const domnode_t* DOMFindChild(const domnode_t* node, const char* label, int idx, const TXProps& attrs, bool recursion)
{
	int i = 0;
	const domnode_t* child = node->child;
	while(child)
	{
		++i;
		if(DOMCheckNode(child, label, attrs) && (-1==idx || i==idx))
		{
			return child;
		}
		else if(recursion && child->child)
		{
			const domnode_t* childchild = DOMFindChild(child, label, idx, attrs, recursion);
			if(childchild)
				return childchild;
		}

		child = (child->next==node->child)?NULL:child->next;
	}
	return NULL;
}

static const domnode_t* DOMFindElement(const domnode_t* node, const char* name, bool recursion)
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
	else if(0 == strcmp("$", name)) // text node
	{
		const domnode_t* child = node->child;
		while(child)
		{
			if(NULL==child->name)
				return child;
			child = (child->next==node->child)?NULL:child->next;
		}
		return NULL;
	}
	else
	{
		int idx = 0;
		TXProps attrs;
		std::string label;
		ParseXPathName(name, label, idx, attrs);
		return DOMFindChild(node, label.c_str(), idx, attrs, recursion);
	}
}

static const domnode_t* DOMFindElementByPath(const domnode_t* node, const char* xpath)
{
	assert(node);
	assert(xpath);

	const char* p = strchr(xpath, '/');
	if(NULL == p)
	{
		return DOMFindElement(node, xpath, false);
	}
	else
	{
		std::string name(xpath, p-xpath);
		const domnode_t* child = DOMFindElement(node, name.c_str(), false);
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
			return DOMFindElement(GetContextElement(), path, true);
		
		std::string name(path, p-path);
		const domnode_t* node = DOMFindElement(GetContextElement(), name.c_str(), true);
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
	//if(child && !child->name)
	//{
	//	value = child->padding?child->padding:"";
	//	return true;
	//}
	while(child)
	{
		if(!child->name)
		{
			value = child->padding?child->padding:"";
			return true;
		}

		child = (child->next==node->child)?NULL:child->next;
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
