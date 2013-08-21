#include "domutil.h"
#include "domnode.h"
#include "domparser.h"
#include "dommodule.h"

// <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

static int identify(const char* p)
{
	return domutil_cmptagname(p, "meta");
}

static int get_content_encoding(const char* content, char* encoding)
{
	const char *c, *p;
	c = strstr(content, "charset");
	if(c)
	{
		c = domutil_skip(c+7);
		if('=' == *c)
		{
			c = domutil_skip(c+1);
			p = domutil_tokenvalue(c);
			if(p > c && p-c < 32)
			{
				strncpy(encoding, c, p-c);
				encoding[p-c] = '\0';
				return 1;
			}
		}
	}
	return 0;
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	char encoding[32];
	const char* p1;
	const char* content;
	p1 = domnode_parse(doc, node, p);

	// http-equiv
	//attr = domnode_find_attr(node, "http-equiv");

	// keyword
	//attr = domnode_find_attr(node, "keyword");

	// content
	content = domnode_attr_find(node, "content");
	if(content)
	{
		// set document encoding
		if(get_content_encoding(content, encoding))
			domparser_setencoding(doc, encoding);
	}

	return p1;
}

int dommetereg()
{
	static dommodule_t s_module;
	memset(&s_module, 0, sizeof(dommodule_t));

	s_module.identify = identify;
	s_module.parse = parse;

	return dommodule_register(&s_module);
}
