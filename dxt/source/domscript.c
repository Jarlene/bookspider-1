#include "domnode.h"
#include "domutil.h"
#include "domparser.h"
#include "dommodule.h"

static int identify(const char* p)
{
	return domutil_cmptagname(p, "script");
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* p1;

	p1 = p;
	while(p1 && *p1 && 0!=strnicmp(p1, "</script>", 9) )
		++p1;

	if(p1)
	{
		p1 += 9;
		domnode_setpadding(node, p, p1-p);
	}
	else
	{
		domparser_seterror(doc, p, "Parse script node error: can't find tag end(</script>)");
	}
	return p1;
}

int domscriptreg()
{
	static dommodule_t s_module;
	memset(&s_module, 0, sizeof(dommodule_t));

	s_module.identify = identify;
	s_module.parse = parse;

	return dommodule_register(&s_module);
}
