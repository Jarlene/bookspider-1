#include "domattr.h"
#include "domutil.h"
#include "domparser.h"
#include <assert.h>

domattr_t* domattr_create()
{
	domattr_t* attr = (domattr_t*)malloc(sizeof(domattr_t));
	if(!attr)
		return NULL;

	memset(attr, 0, sizeof(domattr_t));
	return attr;
}

int domattr_destroy(domattr_t* attr)
{
	FREE(attr->name);
	FREE(attr->value);
	free(attr);
	return 0;
}

int domattr_setname(domattr_t* attr, const char* name, int nameLen)
{
	FREE(attr->name);
	attr->name = domutil_strdup(name, nameLen);
	return attr->name?0:DOMERR_MEMORY;
}

int domattr_setvalue(domattr_t* attr, const char* value, int valueLen)
{
	FREE(attr->value);
	attr->value = domutil_strdup(valueLen?value:"", valueLen); // if valueLen is 0, strdump will copy all string
	return attr->value?0:DOMERR_MEMORY;
}

// parse attribute
// parse type="checked"
// parse type='checked'
// parse type=checked
const char* domattr_parse(const char* p, domattr_t* attr)
{
	const char *name, *value;

	// parse name
	name = domutil_skip(p);
	p = domutil_tokenname(name);
	if(p <= name)
		return p; // <meta keyword="abc""def" link="">

	domattr_setname(attr, name, p-name);

	// parse =
	p = domutil_skip(p);
	if('=' != *p)
		return p; // alone attribute: <input type="checked" checked />

	// parse value
	value = domutil_skip(p+1);
	p = domutil_tokenvalue(value);
	if(p <= value)
		return p;

	switch(*value)
	{
	case '\'':
		attr->eattr = EATTR_SINGLE_QUOTES;
	case '\"':
		attr->eattr = EATTR_DOUBLE_QUOTES;

		assert(p >= value+2);
		domattr_setvalue(attr, value+1, p-value-2);
		break;

	default:
		attr->eattr = EATTR_VOID;
		domattr_setvalue(attr, value, p-value);
	}
	
	return p;
}
