#include "book-spider.h"
#include "cstringext.h"
#include "sys/system.h"
#include "libct/auto_ptr.h"
#include "http.h"
#include "dxt.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include "tools.h"
#include <assert.h>
#include <string>

static int Parse(const char* xml, IBookSpider::OnBook callback, void* param)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

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
		callback(param, 
			UTF8Encode(name.c_str(), encoding), 
			UTF8Encode(author.c_str(), encoding),
			UTF8Encode(uri.c_str(), encoding),
			UTF8Encode(chapter.c_str(), encoding),
			UTF8Encode(datetime.c_str(), encoding));
	}

	return 0;
}

static int Http(const char* uri, const char* req, void** reply)
{
	int r = -1;
	for(int i=0; r<0 && i<20; i++)
	{
		r = http_request(uri, req, reply);
		if(r < 0)
		{
			printf("get %s error: %d\n", uri, r);
			system_sleep(10);
		}
	}
	return r;
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

	libct::auto_ptr<char> reply;
	int r = Http(uri, req, (void**)&reply);
	if(r < 0)
	{
		printf("Check update error: %d\n", r);
		return r;
	}

	libct::auto_ptr<char> result;
	r = DxTransformHtml(&result, reply, xmlfile.c_str());
	if(r < 0)
	{
		tools_write("e:\\a.html", reply, strlen(reply));
		printf("SearchBook[%s]: error: %d.\n", spider->GetName(), r);
		return r;
	}

	return Parse(result, callback, param);
}
