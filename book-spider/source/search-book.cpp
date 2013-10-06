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
#include <assert.h>
#include <string>

static int Parse(const char* xml, const char* book, const char* author, char* bookUri)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	for(bool i=parser.Foreach("books/book"); i; i=parser.Next())
	{
		std::string bookName, bookAuthor, uri/*, chapter*/;
		if(!parser.GetValue("name", bookName) 
			|| !parser.GetValue("author", bookAuthor)
			|| !parser.GetValue("uri", uri))
			continue;

		//parser.GetValue("chapter", chapter);

		// to utf-8
		const char* encoding = parser.GetEncoding();
		if(strieq(UTF8Encode(bookName.c_str(), encoding), book) 
			&& strieq(UTF8Encode(bookAuthor.c_str(), encoding), author))
		{
			strcpy(bookUri, UTF8Encode(uri.c_str(), encoding));
			return 0; // find it
		}
	}

	return ERROR_NOTFOUND; // not found
}

static int Http(const char* uri, const char* req, void** reply)
{
	int r = -1;
	for(int i = 0; r < 0 && i < 5; i++)
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

int SearchBook(const IBookSpider* spider, const char* uri, const char* req, const char* book, const char* author, char* bookUri)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "search/%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("SearchBook: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	libct::auto_ptr<char> reply;
	int r = Http(uri, req, (void**)&reply);
	if(r >= 0)
	{
		libct::auto_ptr<char> result;
		r = DxTransformHtml(&result, reply, xmlfile.c_str());
		if(r < 0)
		{
			printf("SearchBook[%s]: error: %d.\n", spider->GetName(), r);
			return r;
		}

		return Parse(result, book, author, bookUri);
	}
	else if(ERROR_HTTP_REDIRECT == r)
	{
		strcpy(bookUri, reply);
		return 0;
	}
	else
	{
		assert(r < 0);
		printf("SearchBook[%s]: error: %d.\n", spider->GetName(), r);
		return r;
	}
}
