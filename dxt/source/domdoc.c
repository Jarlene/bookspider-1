#include "domdoc.h"
#include "domutil.h"
#include "domnode.h"
#include "domparser.h"
#include <assert.h>

void* domdoc_create(const char* p)
{
	domdoc_t* doc = domparser_parse(p);
	return doc;
}

void domdoc_destroy(void* dom)
{
	domdoc_t* doc = (domdoc_t*)dom;
	FREE(doc->error);
	FREE(doc->errpos);

	if(doc->root)
		domnode_destroy(doc->root);

	FREE(doc);
}

const char* domdoc_getencoding(void* dom)
{
	domdoc_t* doc = (domdoc_t*)dom;
	return doc->encoding;
}

const char* domdoc_geterror(void* dom)
{
	domdoc_t* doc = (domdoc_t*)dom;
	return doc->error;
}
