#ifndef _dom_h_
#define _dom_h_

#include <list>
#include <vector>
#include "ISource.h"
extern "C"
{
#include "domparser.h"
#include "domnode.h"
}

class dom : public ISource
{
public:
	dom(void* doc);
	~dom();

public:
	virtual bool Foreach(const char* path);
	virtual bool Foreach(const char* path, const char* /*match*/){ return Foreach(path); }
	virtual bool ForeachNext();

	virtual bool GetName(const char* path, std::string& name) const;

	virtual bool GetValue(const char* path, std::string& value) const;
	virtual bool GetValue(const char* path, const char* /*match*/, std::string& value) const{ return GetValue(path, value); }

	virtual bool GetAttr(const char* path, const char* name, std::string& value) const;

private:
	bool GetName(const domnode_t* node, std::string& name) const;
	bool GetValue(const domnode_t* node, std::string& value) const;

	bool GetAttr(const domnode_t* node, const char* name, int& value) const;
	bool GetAttr(const domnode_t* node, const char* name, bool& value) const;
	bool GetAttr(const domnode_t* node, const char* name, double& value) const;
	bool GetAttr(const domnode_t* node, const char* name, std::string& value) const;

	bool GetTitle(const domnode_t* node, std::string& title) const{ return GetAttr(node, "title", title); }
	bool GetClass(const domnode_t* node, std::string& style) const{ return GetAttr(node, "class", style); }

private:
	const domnode_t* FindElement(const char* path) const;

private:
	const domnode_t* GetContextElement() const;

private:
	domdoc_t* m_doc;

	typedef std::list<const domnode_t*> TPaths;
	TPaths m_paths;
};

#endif /* !_dom_h_ */
