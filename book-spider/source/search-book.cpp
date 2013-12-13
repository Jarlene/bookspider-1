#include "book-spider.h"
#include "cstringext.h"
#include "web-translate.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include <assert.h>
#include <string>

struct TSearchParam
{
	const char* book;
	const char* author;
	char* bookUri;
};

static int Parse(void *param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	TSearchParam *p = (TSearchParam*)param;
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
		if(strieq(UTF8Encode(bookName.c_str(), encoding), p->book) 
			&& strieq(UTF8Encode(bookAuthor.c_str(), encoding), p->author))
		{
			strcpy(p->bookUri, UTF8Encode(uri.c_str(), encoding));
			return 0; // find it
		}
	}

	return ERROR_NOTFOUND; // not found
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

	TSearchParam p = {book, author, bookUri};
	int r = web_translate(uri, req, xmlfile.c_str(), Parse, &p);
	if(ERROR_HTTP_REDIRECT == r)
	{
		assert(false);
		//strcpy(bookUri, reply);
		return 0;
	}
	else
	{
		assert(r < 0);
		printf("SearchBook[%s]: error: %d.\n", spider->GetName(), r);
		return r;
	}
}
