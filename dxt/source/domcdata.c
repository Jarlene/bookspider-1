#include "domutil.h"
#include "domnode.h"
#include "domparser.h"
#include "dommodule.h"

// <![CDATA[ ----------------------------- ]]>

static int identify(const char* p)
{
	p = domutil_skip(p);
	if('<' != *p)
		return 0;

	p = domutil_skip(p+1);
	return strnicmp(p, "![CDATA[", 8)?0:1;
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* p1;
	p1 = strstr(p, "]]>");
	if(p1)
	{
		p1 += 3;
		domnode_setpadding(node, p, p1-p);
	}
	else
	{
		domparser_seterror(doc, p, "Parse CDATA node error: can't find tag end(]]>).");
	}
	return p1;
}

int domcdatareg()
{
	static dommodule_t s_module;
	memset(&s_module, 0, sizeof(dommodule_t));

	s_module.identify = identify;
	s_module.parse = parse;

	return dommodule_register(&s_module);
}
