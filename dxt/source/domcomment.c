#include "domutil.h"
#include "domnode.h"
#include "domparser.h"
#include "dommodule.h"
#include <assert.h>

// <!-- comment -->

static int identify(const char* p)
{
	p = domutil_skip(p);
	if('<' != *p)
		return 0;

	p = domutil_skip(p);
	return strnicmp(p, "!--", 3)?0:1;
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* p1;
	p = strstr(p, "!--");
	assert(p);
	p1 = strstr(p+3, "-->");
	if(p1)
	{
		p1 += 3;
		domnode_setpadding(node, p, p1-p);
	}
	else
	{
		domparser_seterror(doc, p, "Parse comment node error: can't find tag end(-->).");
	}
	return p1;
}

int domcommentreg()
{
	static dommodule_t s_module;
	memset(&s_module, 0, sizeof(dommodule_t));

	s_module.identify = identify;
	s_module.parse = parse;

	return dommodule_register(&s_module);
}
