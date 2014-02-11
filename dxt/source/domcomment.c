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

	p = domutil_skip(p+1);
	return 0==strncmp(p, "!--", 3) ? 1 : 0;
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* p0, *p1;
	p0 = strstr(p, "!--");
	assert(p0);
	p1 = strstr(p0+3, "-->");
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
