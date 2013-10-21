#include "JokeSpider.h"
#include "cstringext.h"
#include "http-translate.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include "tools.h"
#include <assert.h>
#include <string>

struct TJokeParam
{
	OnJoke callback;
	void* param;
};

static int joke_get_helper(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	TJokeParam* p = (TJokeParam*)param;
	for(bool i=parser.Foreach("jokes/joke"); i; i=parser.Next())
	{
		int approve = 0;
		int disapprove = 0;
		std::string id, author, datetime, content, image;
		if(!parser.GetValue("id", id)
			|| !parser.GetValue("content", content)
			|| !parser.GetValue("approve", approve)
			|| !parser.GetValue("disapprove", disapprove))
			continue;

		// check valid
		if(id.empty() || content.empty())
			continue;

		parser.GetValue("image", image);
		parser.GetValue("author", author);
		parser.GetValue("datetime", datetime);

		// 13-03-07 21:03 => 2013-03-07 21:03
		if(datetime.length() == 14)
			datetime.insert(0, "20");
		else if(datetime.length() == 11)
			datetime.insert(0, "2013-");

		// to utf-8
		const char* encoding = parser.GetEncoding();
		p->callback(p->param, 
			UTF8Encode(id.c_str(), encoding), 
			UTF8Encode(author.c_str(), encoding), 
			UTF8Encode(datetime.c_str(), encoding),
			UTF8Encode(content.c_str(), encoding),
			UTF8Encode(image.c_str(), encoding),
			approve, disapprove);
	}

	return 0;
}

int joke_get(const IJokeSpider* spider, const char* uri, const char* req, OnJoke callback, void* param)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "joke-%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("ListJoke: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	TJokeParam p = {callback, param};
	return http_translate(uri, req, xmlfile.c_str(), joke_get_helper, &p);
}

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
	for(bool i=parser.Foreach("comments/comment"); i; i=parser.Next())
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
	return http_translate(uri, req, xmlfile.c_str(), joke_comment_helper, &p);
}
