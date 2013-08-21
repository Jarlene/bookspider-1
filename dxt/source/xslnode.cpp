extern "C"
{
#include "domutil.h"
#include "domattr.h"
#include "domnode.h"
#include "domparser.h"
}
#include <assert.h>
#include <string>
#include "ISource.h"

int domdoc_translate_node(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output);

static bool xslvalueof(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	// <xsl:value-of select="xpath"> </xsl:value-of>
	// <xsl:value-of select="xpath" match="command"> </xsl:value-of>
	std::string value;

	const char* xpath = domnode_attr_find(node, "select");
	if(!xpath)
	{
		domparser_seterror(doc, node->name, "xsl:value-of can't found 'select' attribute.");
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
		//domparser_seterror(doc, node->name, "xsl:value-of can't found xpath");
		return false;
	}

	output += value;
	return true;
}

static bool xslattrof(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	// <xsl:attr-of select="xpath" attr="name"> </xsl:attr-of>
	const char* xpath = domnode_attr_find(node, "select");
	if(!xpath)
	{
		domparser_seterror(doc, node->name, "xsl:attr-of can't found 'select' attribute.");
		return false;
	}

	const char* xattr = domnode_attr_find(node, "attr");
	if(!xattr)
	{
		domparser_seterror(doc, node->name, "xsl:attr-of can't found 'attr' attribute.");
		return false;
	}

	std::string value;
	if(!xml->GetAttr(xpath, xattr, value))
	{
		//domparser_seterror(doc, node->name, "xsl:attr-of can't get attribute value.");
		return false;
	}

	output += value;
	return true;
}

static bool xslelse(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	const domnode_t* child = node->child;
	while(child)
	{
		domdoc_translate_node(doc, xml, child, output);
		child = (child->next==node->child)?NULL:child->next;
	}

	return true; // match
}

static bool xslif(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	// <xsl:if select="xpath"> </xsl:if>
	// <xsl:if select="xpath" name="value"> </xsl:if>
	// <xsl:if select="xpath" attr="value"> </xsl:if>
	const char* xpath = domnode_attr_find(node, "select");
	if(!xpath)
	{
		domparser_seterror(doc, node->name, "xsl:if can't found 'select' attribute.");
		return false;
	}

	std::string value;
	const char* name = domnode_attr_find(node, "name");
	if(name)
	{
		if(!xml->GetName(xpath, value))
			return false;

		if(0 != stricmp(name, value.c_str()))
			return false;
	}
	else
	{
		const domattr_t* attr = node->attr;
		while(attr)
		{
			assert(attr->name);
			if(0!=stricmp("select", attr->name))
			{
				if(!xml->GetAttr(xpath, attr->name, value))
					return false;

				if(0 != stricmp(attr->value, value.c_str()))
					return false;
				break;
			}
			attr = (attr->next==node->attr)?NULL:attr->next;
		}

		if(!attr)
			return false;
	}

	return xslelse(doc, xml, node, output);
}

static bool xslforeach(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	// <xsl:for-each select="xpath"> </xsl:for-each>
	// <xsl:for-each select="xpath" match="command"> </xsl:for-each>
	const char* xpath = domnode_attr_find(node, "select");
	if(!xpath)
	{
		domparser_seterror(doc, node->name, "xsl:for-each can't found 'select' attribute.");
		return false;
	}

	bool v = false;
	const char* xmatch = domnode_attr_find(node, "match");
	if(xmatch)
		v = xml->Foreach(xpath, xmatch);
	else
		v = xml->Foreach(xpath);

	if(!v)
	{
		//domparser_seterror(doc, node->name, "xsl:for-each can't found xpath");
		return false;
	}

	do
	{
		xslelse(doc, xml, node, output);
	} while (xml->ForeachNext());
	return true;
}

static bool xslchoose(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	// <xsl:choose>
	//		<xsl:when>
	//		</xsl:when>
	//		<xsl:when>
	//		</xsl:when>
	//		<xsl:otherwise>
	//		</xsl:otherwise>
	// </xsl:choose>
	const domnode_t* child = node->child;
	while(child)
	{
		if(child->name)
		{
			if(0 == stricmp("xsl:when", child->name))
			{
				assert(child->end > (const domnode_t*)ETAG_END);
				if(xslif(doc, xml, child, output))
					break;
			}
			else if(0 == stricmp("xsl:otherwise", child->name))
			{
				assert(child->end > (const domnode_t*)ETAG_END);
				xslelse(doc, xml, child, output);
			}
		}
		child = (child->next==node->child)?NULL:child->next;
	}
	return true;
}

static bool xslselect(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	// <xsl:select select="" match="">
	//		<option>text</option>
	//		<option value="">text</option>
	// </xsl:select>
	std::string xvalue;
	const char* xpath = domnode_attr_find(node, "select");
	if(xpath)
	{
		const char* xmatch = domnode_attr_find(node, "match");
		if(xmatch)
			xml->GetValue(xpath, xmatch, xvalue);
		else
			xml->GetValue(xpath, xvalue);
	}

	const domnode_t* child = node->child;
	while(child)
	{
		if(child->name && 0==stricmp("option", child->name))
		{
			assert(child->end > (const domnode_t*)ETAG_END);
			const char* value = domnode_attr_find(child, "value");
			if(!value)
			{
				// don't have value attribute
				// use option text instead
				const domnode_t* textnode = child->child;
				if(textnode && !textnode->name)
				{
					assert(textnode->padding);
					value = textnode->padding;
				}
			}

			if(value && 0==stricmp(value, xvalue.c_str()))
			{
				domattr_t* attr = domattr_create();
				if(attr)
				{
					attr->eattr = EATTR_DOUBLE_QUOTES;
					domattr_setname(attr, "selected", 0);
					domattr_setvalue(attr, "selected", 0);
					domnode_attr_append((domnode_t*) node, attr);
					domdoc_translate_node(doc, xml, child, output);
					domnode_attr_delete((domnode_t*) node, attr);
					domattr_destroy(attr);
				}
			}
			else
			{
				domdoc_translate_node(doc, xml, child, output);
			}
		}

		child = (child->next==node->child)?NULL:child->next;
	}
	return true;
}

bool xslnode_translate(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	if(!node->name)
		return false;

	if(0 == stricmp("xsl:value-of", node->name))
	{
		xslvalueof(doc, xml, node, output);
	}
	else if(0 == stricmp("xsl:attr-of", node->name))
	{
		xslattrof(doc, xml, node, output);
	}
	else if(0 == stricmp("xsl:for-each", node->name))
	{
		xslforeach(doc, xml, node, output);
	}
	else if(0 == stricmp("xsl:if", node->name))
	{
		xslif(doc, xml, node, output);
	}
	else if(0 == stricmp("xsl:choose", node->name))
	{
		xslchoose(doc, xml, node, output);
	}
	else if(0 == stricmp("xsl:select", node->name))
	{
		xslselect(doc, xml, node, output);
	}
	else
	{
		return false;
	}
	return true;
}
