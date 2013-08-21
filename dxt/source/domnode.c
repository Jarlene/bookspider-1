#include "domattr.h"
#include "domnode.h"
#include "domutil.h"
#include "domparser.h"
#include <assert.h>

void domnode_append(domnode_t* parent, domnode_t* node)
{
	assert(parent);
	assert(!node->parent);
	assert(!node->prev);
	assert(!node->next);

	if(parent->child)
	{
		node->next = parent->child;
		node->prev = parent->child->prev;
		parent->child->prev = node;
		node->prev->next = node;
	}
	else
	{
		parent->child = node;
		node->next = node;
		node->prev = node;
	}

	node->parent = parent;
	assert(node->next == parent->child);
}

int domnode_setname(domnode_t* node, const char* name, int nameLen)
{
	FREE(node->name);
	node->name = domutil_strdup(name, nameLen);
	return node->name?0:DOMERR_MEMORY;
}

int domnode_setpadding(domnode_t* node, const char* padding, int paddingLen)
{
	FREE(node->padding);
	node->padding = domutil_strdup(padding, paddingLen);
	return node->padding?0:DOMERR_MEMORY;
}

static void domnode_destroychildren(domnode_t* node)
{
	domnode_t *child, *nextchild;

	nextchild = NULL;
	child = node->child;
	while(nextchild!=node->child)
	{
		nextchild = child->next;
		domnode_destroy(child);
		child = nextchild;
	}
}

static void domnode_destroyattributes(domnode_t* node)
{
	domattr_t *attr, *nextattr;

	nextattr = NULL;
	attr = node->attr;
	while(nextattr != node->attr)
	{
		nextattr = attr->next;
		FREE(attr->name);
		FREE(attr->value);
		FREE(attr);
		attr = nextattr;
	}
}

int domnode_destroy(domnode_t* node)
{
	// destroy all child
	domnode_destroychildren(node);

	// destroy all attribute(s)
	domnode_destroyattributes(node);

	FREE(node->name);
	FREE(node->padding);
	FREE(node);
	return 0;
}

domnode_t* domnode_create()
{
	domnode_t* node = (domnode_t*)malloc(sizeof(domnode_t));
	if(!node)
		return NULL;

	memset(node, 0, sizeof(domnode_t));
	return node;
}

void domnode_attr_append(domnode_t* node, domattr_t* attr)
{
	assert(node && attr);
	if(node->attr)
	{
		attr->next = node->attr;
		attr->prev = node->attr->prev;
		attr->prev->next = attr;
		node->attr->prev = attr;
	}
	else
	{
		node->attr = attr;
		attr->next = attr;
		attr->prev = attr;
	}
}

void domnode_attr_delete(domnode_t* node, domattr_t* attr)
{
	assert(node && attr);
	if(node->attr == attr)
		node->attr = (attr==attr->next)?NULL:attr->next;
	attr->next->prev = attr->prev;
	attr->prev->next = attr->next;
}

const char* domnode_attr_find(const domnode_t* node, const char* name)
{
	domattr_t* attr;
	assert(node && name);
	attr = node->attr;
	while(attr)
	{
		if(attr->name && 0==stricmp(attr->name, name))
			return attr->value;
		attr = (attr->next==node->attr)?NULL:attr->next;
	}

	return NULL;
}

// parse <meta keyword="abc" link="http://abc.com" />
// => (keyword, "abc"), (link, "httpd://abc.com")
// => p = "/>"

#define IS_TAGEND(p) (*p=='>' || (('?'==*p||'/'==*p) && p[1]=='>'))
const char* domnode_attr_parse(domnode_t* node, const char* p)
{
	domattr_t *attr;
	const char *p1;
	assert(node && p);
	p = domutil_skip(p);

	// end with
	//	 1.<html ... >
	//   2.<html ... />
	//   3.<?xml ... ?>
	while(*p && !IS_TAGEND(p))
	{
		attr = domattr_create();
		if(!attr)
			return NULL;

		p1 = domattr_parse(p, attr);
		if(p1 == p)
		{
			// attribute error, skip it
			p1 = domutil_tokenvalue(p);
			domattr_destroy(attr);
		}
		else
		{
			assert(attr->name);
			domnode_attr_append(node, attr);
		}
		p = domutil_skip(p1);
	}

	return p;
}

// parse <title>abcd</title> => abcd
static const char* domnode_parsetext(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* padding;

	assert(p);
	padding = p;
	while(*p && '<'!=*p)
		++p;

	assert(p > padding);
	node->end = (domnode_t*)ETAG_END;
	domnode_setpadding(node, padding, p-padding);
	return p;
}

// -------------------| padding |--|name|---|   attribute   |---
// <html><head></head>            < body     onload="OnLoad() >
const char* domnode_parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char *name, *padding;

	padding = p;
	p = domutil_skip(p);
	if('<' != *p)
	{
		// text node
		p = domnode_parsetext(doc, node, padding);
	}
	else
	{
		// padding
		if(p > padding)
			domnode_setpadding(node, padding, p-padding);

		// name
		name = domutil_skip(p+1);
		if('/'==*name)
		{
			// end tag
			assert(0 == node->end);
			node->end = (domnode_t*)ETAG_END;

			name = domutil_skip(name+1);
		}

		p = domutil_tokenname(name);
		if(p <= name)
		{
			domparser_seterror(doc, padding, "tag end is empty.");
			return NULL;
		}

		domnode_setname(node, name, p-name);

		// attributes
		p = domnode_attr_parse(node, p);

		// end
		assert(0==*p || IS_TAGEND(p));
		if('/' == *p || '?' == *p)
		{
			assert(0 == node->end);
			node->end = (domnode_t*)ETAG_SELF;
			p = domutil_skip(p+1);
		}
		assert('>' == *p);
		p = strchr(p, '>');
		if(!p)
			domparser_seterror(doc, padding, "tag don't end with '>'");
		else
			p += 1;
	}

	return p;
}
