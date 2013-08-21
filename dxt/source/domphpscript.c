#include "domutil.h"
#include "domnode.h"
#include "domparser.h"
#include "dommodule.h"

// <?php ... ?>

static int identify(const char* p)
{
	return domutil_cmptagname(p, "?php") ;
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* p1;

	p1 = strstr(p, "?>");
	if(p1)
	{
		p1 += 2;
		domnode_setpadding(node, p, p1-p);
	}
	else
	{
		domparser_seterror(doc, p, "Parse php-script node error: can't find end tag(</script>)");
	}
	return p1;
}

int domphpscriptreg()
{
	static dommodule_t s_module;
	memset(&s_module, 0, sizeof(dommodule_t));

	s_module.identify = identify;
	s_module.parse = parse;

	return dommodule_register(&s_module);
}
