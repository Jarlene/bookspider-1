#include "JokeSpider.h"
#include "cstringext.h"
#include "web-translate.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include <assert.h>
#include <string>

struct TCommentParam
{
	OnComment callback;
	void* param;
};

static int joke_comment_helper(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	TCommentParam* p = (TCommentParam*)param;
	for(bool i=parser.Foreach("comment"); i; i=parser.Next())
	{
		std::string icon, user, content;
		if(!parser.GetValue("content", content))
			continue;

		// check valid
		if(content.empty())
			continue;

		parser.GetValue("icon", icon);
		parser.GetValue("user", user);

		// to utf-8
		const char* encoding = parser.GetEncoding();
		p->callback(p->param, 
			UTF8Encode(icon.c_str(), encoding), 
			UTF8Encode(user.c_str(), encoding), 
			UTF8Encode(content.c_str(), encoding));
	}

	return 0;
}

int joke_comment(const IJokeSpider* spider, const char* uri, const char* req, OnComment callback, void* param)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "comment-%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("joke_comment: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	TCommentParam p = {callback, param};
	return web_translate(uri, req, xmlfile.c_str(), joke_comment_helper, &p);
}
