#include "domparser.h"
#include "domutil.h"
#include "domattr.h"
#include "domnode.h"
#include "dommodule.h"
#include "domdoc.h"
#include <assert.h>

domdoc_t* domparser_parse(const char* p)
{
	domdoc_t* doc;
	domnode_t* node;
	dommodule_t* module; 

	doc = (domdoc_t*)malloc(sizeof(domdoc_t));
	if(!doc)
		return NULL;
	
	memset(doc, 0, sizeof(domdoc_t));
	doc->p = p;
	doc->root = domnode_create();
	if(!doc->root)
	{
		domdoc_destroy(doc);
		return NULL;
	}

	assert(p);
	p = domutil_skipbom(p);
	while(*p)
	{
		node = domnode_create();
		if(!node)
		{
			domdoc_destroy(doc);
			return NULL;
		}

		module = dommodule_identify(p);
		if(module)
			p = module->parse(doc, node, p);
		else
			p = domnode_parse(doc, node, p);

		if(!p)
		{
			// mark error and break
			domnode_destroy(node);
			break;
		}

		domparser_append(doc->root, node); // auto-indent
	}

	return doc;
}

static void domparser_rearrange(domnode_t* node)
{
	domnode_t* parent = node->parent;
	domnode_t* sibling = node->next;
	assert(parent && sibling);

	// re-parent
	while(sibling != parent->child)
	{
		sibling->parent = node;
		sibling = sibling->next;
	}

	// re-list children
	assert(0 == node->child);
	if(node->next != parent->child)
	{
		node->child = node->next;
		node->child->prev = parent->child->prev;
		node->child->prev->next = node->child;
	}

	// re-list parent
	node->next = parent->child;
	parent->child->prev = node;
}

static void domparser_rearrange_ul(domnode_t* parent)
{
	// ul -> li: miss </li>
	domnode_t* next;
	domnode_t* node;
	domnode_t* child = parent->child;
	while(child)
	{
		if(child->name && 0==stricmp(child->name, "li"))
		{
			if(0==child->end && 0==child->child)
			{
				// find next li node
				next = child->next;
				while(next != parent->child && (NULL==next->name || stricmp(next->name, "li")))
					next = next->next;

				node = child->next;
				while(node != next)
				{
					node->parent = child;
					node = node->next;
				}

				// re-list children
				assert(0 == child->child);
				if(child->next != next)
				{
					child->child = child->next;
					child->child->prev = next->prev;
					child->child->prev->next = child->child;
				}

				// re-list parent
				child->next = next;
				next->prev = child;
			}
		}

		child = child->next;
		if(child == parent->child)
			break;
	}
}

int domparser_append(domnode_t* parent, domnode_t* node)
{
	domnode_t* child = parent->child;
	assert(parent && node);

	if(!node->name || node->end!=(domnode_t*)ETAG_END)
	{
		domnode_append(parent, node);
		return 0;
	}
	else
	{
		while(child)
		{
			assert(node->name);
			child = child->prev; // last child node
			if(0==child->end && child->name && 0==stricmp(child->name, node->name))
			{
				assert(0 == child->child);
				child->end = node; // endtag
				domparser_rearrange(child);

				// rearrange ul -> li node
				if(0 == stricmp(child->name, "ul"))
					domparser_rearrange_ul(child);
				return 0;
			}

			if(child == parent->child)
				break;
		}

		domnode_append(parent, node);
		return 0;
	}
}

void domparser_seterror(domdoc_t* doc, const char* perr, const char* errmsg)
{
	int row = 0;
	int col = 0;
	const char* p;
	char buffer[64] = {0};

	assert(doc);
	// only record one error
	if(doc->perr)
		return;

	for(p=doc->p; *p && p < perr; ++p)
	{
		switch (*p) {
			case '\r':
				col = 0;
				if (p[1] == '\n')
				{
					++row;
					++p;
				}
				break;

			case '\n':
				++row;
				col = 0;
				if (p[1] == '\r')
					++p;
				break;

			default:
				++col;
				break;
		}
	}

	_snprintf(buffer, sizeof(buffer)-1, "Line: %d, Col: %d. ", row+1, col+1);

	doc->perr = perr;
	doc->error = domutil_strdup(errmsg, 0);
	doc->errpos = domutil_strdup(buffer, 0);
}

void domparser_setencoding(domdoc_t* doc, const char* encoding)
{
	FREE(doc->encoding);
	doc->encoding = domutil_strdup(encoding, 0);
}
