#include "domutil.h"
#include "domnode.h"
#include "domparser.h"
#include "dommodule.h"

// <?xml version="1.0" encoding="utf-8"?>

static int identify(const char* p)
{
	return domutil_cmptagname(p, "?xml") ;
}

static const char* parse(domdoc_t* doc, domnode_t* node, const char* p)
{
	const char* p1;
	const char* encoding;
	p1 = domnode_parse(doc, node, p);

	// version
	//attr = domnode_find_attr(node, "version");

	// encoding
	encoding = domnode_attr_find(node, "encoding");
	if(encoding)
	{
		// set document encoding
		domparser_setencoding(doc, encoding);
	}

	return p1;
}

int xmldeclarationreg()
{
	static dommodule_t s_module;
	memset(&s_module, 0, sizeof(dommodule_t));

	s_module.identify = identify;
	s_module.parse = parse;

	return dommodule_register(&s_module);
}
