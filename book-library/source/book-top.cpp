#include "book-site.h"
#include "sys/system.h"
#include "libct/auto_ptr.h"
#include "utf8codec.h"
#include "XMLParser.h"
#include "config.h"
#include "tools.h"
#include "http-translate.h"

static int GetBookId(const char* uri)
{
	const char* p = strrchr(uri, '/');
	return p ? atoi(p+1) : atoi(uri);
}

struct TParam
{
	IBookSite* site;
	book_site_spider_fcb callback;
	void* param;
};

static int OnListBook(void* param, const char* xml)
{
	TParam *p = (TParam*)param;

	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	for(bool i=parser.Foreach("book"); i; i=parser.Next())
	{
		std::string name, author, uri, category, chapter, datetime;
		if(!parser.GetValue("name", name) 
			|| !parser.GetValue("author", author)
			|| !parser.GetValue("uri", uri))
			continue;

		// check book valid
		if(name.empty() || author.empty() || uri.empty())
			continue;

		parser.GetValue("category", category);
		parser.GetValue("chapter", chapter);
		parser.GetValue("datetime", datetime);

		const char* encoding = parser.GetEncoding();

		// qidian/zongheng: 13-03-07 21:03 => 2013-03-07 21:03
		if(datetime.length() == 14)
			datetime.insert(0, "20");

		// zongheng: [category] => category
		if('['==category.at(0) && ']'==category.at(category.length()-1))
		{
			category.erase(0, 1);
			category.erase(category.length()-1, 1);
		}

		// data filter

		book_t book;
		parser.GetValue("vote", book.vote);
		book.id = p->site->GetId() * BOOK_ID + GetBookId(uri.c_str());

		// to utf-8
		strcpy(book.name, UTF8Encode(name.c_str(), encoding));
		strcpy(book.author, UTF8Encode(author.c_str(), encoding));
		strcpy(book.uri, UTF8Encode(uri.c_str(), encoding));
		strcpy(book.category, UTF8Encode(category.c_str(), encoding));
		strcpy(book.chapter, UTF8Encode(chapter.c_str(), encoding));
		strcpy(book.datetime, UTF8Encode(datetime.c_str(), encoding));

		// call-back
		int r = p->callback(p->param, &book);
		if(r)
			return r;
	}
	return 0;
}

int ListBook(IBookSite* site, int top, book_site_spider_fcb callback, void* param)
{
	char uri[256] = {0};
	sprintf(uri, "top/web-%s", site->GetName());

	std::string xmlfile;
	if(!g_config.GetConfig(uri, xmlfile)) // xml file
	{
		printf("TopBook: can't find %s xml file.\n", site->GetName());
		return ERROR_NOT_FOUND;
	}

	int r = 0;
	TParam p = {site, callback, param};
	const char* pattern = site->GetUri(top);
	for(int page = 1; 0 == r; page++)
	{
		sprintf(uri, pattern, page);
		r = http_translate(uri, NULL, xmlfile.c_str(), OnListBook, &p);
		printf("TopBook[%s]: parse page %d: %d.\n", site->GetName(), page, r);
	}

	return r;
}
