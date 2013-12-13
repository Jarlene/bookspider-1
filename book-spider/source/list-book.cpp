#include "book-spider.h"
#include "cstringext.h"
#include "web-translate.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include <assert.h>
#include <string>

struct TListParam
{
	IBookSpider::OnBook callback;
	void* param;
};

static int Parse(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	TListParam* p = (TListParam*)param;
	for(bool i=parser.Foreach("books/book"); i; i=parser.Next())
	{
		std::string name, author, uri, chapter, datetime;
		if(!parser.GetValue("name", name) 
			|| !parser.GetValue("author", author)
			|| !parser.GetValue("uri", uri)
			|| !parser.GetValue("chapter", chapter))
			continue;

		// check book valid
		if(name.empty() || author.empty() || uri.empty())
			continue;

		parser.GetValue("datetime", datetime);
		// 13-03-07 21:03 => 2013-03-07 21:03
		if(datetime.length() == 14)
			datetime.insert(0, "20");
		else if(datetime.length() == 11)
			datetime.insert(0, "2013-");

		// to utf-8
		const char* encoding = parser.GetEncoding();
		p->callback(p->param, 
			UTF8Encode(name.c_str(), encoding), 
			UTF8Encode(author.c_str(), encoding),
			UTF8Encode(uri.c_str(), encoding),
			UTF8Encode(chapter.c_str(), encoding),
			UTF8Encode(datetime.c_str(), encoding));
	}

	return 0;
}

int ListBook(const IBookSpider* spider, const char* uri, const char* req, IBookSpider::OnBook callback, void* param)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "update/web-%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("SearchBook: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	TListParam p = {callback, param};
	return web_translate(uri, req, xmlfile.c_str(), Parse, &p);
}
