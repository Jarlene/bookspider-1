extern "C"
{
#include "domdoc.h"
#include "domparser.h"
}
#include <assert.h>
#include <string>
#include "ISource.h"

bool xslattr_translate(domdoc_t* doc, ISource* xml, const char* value, std::string& output);
bool xslnode_translate(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output);

static void domdoc_translate_attr_value(const domattr_t* attr, const std::string& value, std::string& output)
{
	switch(attr->eattr)
	{
	case EATTR_VOID:
		output += value;
		break;

	case EATTR_SINGLE_QUOTES:
		output += '\'';
		output += value;
		output += '\'';
		break;

	case EATTR_DOUBLE_QUOTES:
		output += '\"';
		output += value;
		output += '\"';
		break;

	default:
		assert(false);
	}
}

static int domdoc_translate_attr(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	const domattr_t* attr;
	for(attr=node->attr; attr; attr=(attr->next==node->attr)?NULL:attr->next)
	{
		assert(attr->name);
		
		std::string value;
		if(attr->value)
		{
			std::string result;
			if(!xslattr_translate(doc, xml, attr->value, result))
				continue;

			domdoc_translate_attr_value(attr, result, value);
			//else
			//{
			//	domparser_seterror(doc, attr->value, "domdoc_translate_attr error");
			//}
		}

		output += ' ';
		output += attr->name;

		if(attr->value)
		{
			output += '=';
			output += value;
		}
	}
	return 0;
}

int domdoc_translate_node(domdoc_t* doc, ISource* xml, const domnode_t* node, std::string& output)
{
	assert(node->name || !node->attr); // => if(!node->name) assert(node->attr);
	output += node->padding?node->padding:"";

	if(!node->name)
		return 0;

	if((const domnode_t*)ETAG_END == node->end)
	{
		output += "</";
		output += node->name;
		output += '>';
	}
	else
	{
		if(xslnode_translate(doc, xml, node, output))
			return 0;

		// self
		output += '<';
		output += node->name;

		domdoc_translate_attr(doc, xml, node, output);

		if((const domnode_t*)ETAG_SELF == node->end)
		{
			output += ('?'==node->name[0])?'?':'/';
			output += '>';
			return 0;
		}

		output += '>';

		// children
		const domnode_t* child = node->child;
		while(child)
		{
			assert(node->end);
			domdoc_translate_node(doc, xml, child, output);
			child = (child->next==node->child)?NULL:child->next;
		}

		// end tag
		if(node->end > (const domnode_t*)ETAG_END)
		{
			assert(node->end->end == (const domnode_t*)ETAG_END);
			domdoc_translate_node(doc, xml, node->end, output);
		}
	}
	return 0;
}

int domdoc_translate(void* dom, ISource* xml, std::string& output)
{
	assert(dom && xml);
	domdoc_t* doc = (domdoc_t*)dom;

	assert(doc->root);
	const domnode_t* child = doc->root->child;
	while(child)
	{
		domdoc_translate_node(doc, xml, child, output);
		child = (child->next==doc->root->child)?NULL:child->next;
	}
	return 0;
}
