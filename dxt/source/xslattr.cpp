extern "C"
{
#include "domutil.h"
#include "domattr.h"
#include "domnode.h"
#include "domparser.h"
}
#include <string>
#include "ISource.h"

static domnode_t* xslattr_parse(domdoc_t* doc, const char* p)
{
	domnode_t* node = domnode_create();
	if(!node)
		return NULL;

	domnode_parse(doc, node, p);
	return node;
}

static bool xslattr_getvalue(domdoc_t* doc, ISource* xml, const char* prop, std::string& value)
{
	domnode_t* node = xslattr_parse(doc, prop);
	if(!node)
		return false;

	const char* xpath = domnode_attr_find(node, "select");
	if(!xpath)
	{
		domnode_destroy(node);
		return false;
	}

	bool v = false;
	const char* xmatch = domnode_attr_find(node, "match");
	if(xmatch)
		v = xml->GetValue(xpath, xmatch, value);
	else
		v = xml->GetValue(xpath, value);

	domnode_destroy(node);
	return v;
}

static bool xslattr_getvalue2(domdoc_t* doc, ISource* xml, const char* prop)
{
	std::string value;
	domnode_t* node = xslattr_parse(doc, prop);
	if(!node)
		return false;

	const char* xpath = domnode_attr_find(node, "select");
	if(!xpath)
	{
		domnode_destroy(node);
		return false;
	}

	bool v = false;
	const char* xmatch = domnode_attr_find(node, "match");
	if(xmatch)
		v = xml->GetValue(xpath, xmatch, value);
	else
		v = xml->GetValue(xpath, value);

	if(!v)
	{
		domnode_destroy(node);
		return false;
	}

	const char* xvalue = domnode_attr_find(node, "value");
	if(xvalue)
		v= 0==stricmp(value.c_str(), xvalue);
	else
		v = 0==stricmp(value.c_str(), "true");

	domnode_destroy(node);
	return v;
}

bool xslattr_translate(domdoc_t* doc, ISource* xml, const char* value, std::string& output)
{
	// handle xsl value
	const char* valueOfTag = "xsl:value-of";
	const char* propChecked = "xsl:prop-checked";
	const char* propDisabled = "xsl:prop-disabled";
	if(domutil_cmptagname(value, valueOfTag))
	{
		std::string xvalue;
		if(!xslattr_getvalue(doc, xml, value, xvalue))
			return false;
		output += xvalue;
		return true;
	}
	else if(domutil_cmptagname(value, propChecked))
	{
		if(!xslattr_getvalue2(doc, xml, value))
			return false;
		output += "checked";
		return true;
	}
	else if(domutil_cmptagname(value, propDisabled))
	{
		if(!xslattr_getvalue2(doc, xml, value))
			return false;
		output += "disabled";
		return true;
	}
	else
	{
		// plain value
		output += value;
		return true;
	}
}
