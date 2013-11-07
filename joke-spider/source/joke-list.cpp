#include "JokeSpider.h"
#include "cstringext.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include "web-translate.h"
#include <assert.h>
#include <string>

struct TJokeParam
{
	OnJoke callback;
	void* param;
};

static int joke_list_parser(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	TJokeParam* p = (TJokeParam*)param;
	for(bool i=parser.Foreach("joke"); i; i=parser.Next())
	{
		int approve = 0;
		int disapprove = 0;
		int comment = 0;
		std::string id, icon, author, datetime, content, image;
		if(!parser.GetValue("id", id)
			//			|| !parser.GetValue("content", content)
			|| !parser.GetValue("approve", approve)
			|| !parser.GetValue("disapprove", disapprove))
			continue;

		for(bool j=parser.Foreach("contents/content"); j; j=parser.Next())
		{
			std::string value;
			if(parser.GetValue(".", value))
			{
				if(value.empty())
					continue;
				if(!content.empty())
					content += "\r\n";
				content += value;
			}
		}

		// check valid
		if(id.empty() || content.empty())
			continue;

		parser.GetValue("icon", icon);
		parser.GetValue("image", image);
		parser.GetValue("author", author);
		parser.GetValue("datetime", datetime);
		parser.GetValue("comment", comment);

		// 13-03-07 21:03 => 2013-03-07 21:03
		if(datetime.length() == 14)
			datetime.insert(0, "20");
		else if(datetime.length() == 11)
			datetime.insert(0, "2013-");

		// to utf-8
		const char* encoding = parser.GetEncoding();
		p->callback(p->param, 
			UTF8Encode(id.c_str(), encoding), 
			UTF8Encode(icon.c_str(), encoding), 
			UTF8Encode(author.c_str(), encoding), 
			UTF8Encode(datetime.c_str(), encoding),
			UTF8Encode(content.c_str(), encoding),
			UTF8Encode(image.c_str(), encoding),
			approve, disapprove, comment);
	}

	return 0;
}

int joke_list(const IJokeSpider* spider, const char* uri, const char* req, OnJoke callback, void* param)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "joke-%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("joke_get: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	TJokeParam p = {callback, param};
	return web_translate(uri, req, xmlfile.c_str(), joke_list_parser, &p);
}
