#include "booksite.h"
#include <string>
#include "sys/system.h"
#include "XMLParser.h"
#include "netmanager.h"
#include "utf8.h"

static const char* g_name = "zongheng";

struct TopUrls
{
	int type;
	std::string url;
};

static TopUrls g_urls[] = { 
	{ ETT_ALL_MARK, "http://book.zongheng.com/store/c0/c0/b9/u4/p%d/v9/s9/t0/ALL.html" },
	{ ETT_ALL_VOTE, "http://book.zongheng.com/store/c0/c0/b9/u2/p%d/v9/s9/t0/ALL.html" },
	{ ETT_MONTH_VOTE, "http://book.zongheng.com/store/c0/c0/b9/u7/p%d/v9/s9/t0/ALL.html" },
	{ ETT_WEEK_VOTE, "http://book.zongheng.com/store/c0/c0/b9/u10/p%d/v9/s9/t0/ALL.html" },
	{ ETT_ALL_VIEW, "http://book.zongheng.com/store/c0/c0/b9/u1/p%d/v9/s9/t0/ALL.html" },
	{ ETT_MONTH_VIEW, "http://book.zongheng.com/store/c0/c0/b9/u6/p%d/v9/s9/t0/ALL.html" },
	{ ETT_WEEK_VIEW, "http://book.zongheng.com/store/c0/c0/b9/u9/p%d/v9/s9/t0/ALL.html" },
};


static int GetTopUrl(int top, std::string& url)
{
	for(size_t i=0; i<sizeof(g_urls)/sizeof(g_urls[0]); i++)
	{
		if(g_urls[i].type == top)
		{
			url = g_urls[i].url;
			return 0;
		}
	}
	return -1;
}

static int ParseXml(const char* xml, book_site_spider_fcb callback, void* param)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	for(bool i=parser.Foreach("books/book"); i; i=parser.Next())
	{
		std::string name, author, uri, category, chapter, datetime;
		if(!parser.GetValue("name", name) 
			|| !parser.GetValue("author", author)
			|| !parser.GetValue("uri", uri))
			continue;

		parser.GetValue("category", category);
		parser.GetValue("chapter", chapter);
		parser.GetValue("datetime", datetime);

		// data filter

		// to utf-8
		book_t book;
		book.siteid = 2;
		parser.GetValue("vote", book.vote);

		const char* encoding = parser.GetEncoding();
		to_utf8(name.c_str(), encoding, book.name, sizeof(book.name));
		to_utf8(author.c_str(), encoding, book.author, sizeof(book.author));
		to_utf8(uri.c_str(), encoding, book.uri, sizeof(book.uri));
		to_utf8(category.c_str(), encoding, book.category, sizeof(book.category));
		to_utf8(chapter.c_str(), encoding, book.chapter, sizeof(book.chapter));
		to_utf8(datetime.c_str(), encoding, book.datetime, sizeof(book.datetime));

		// call-back
		int r = callback(param, &book);
		if(r)
			return r;
	}
	return 0;
}

int Spider(int top, book_site_spider_fcb callback, void* param)
{
	char uri[256] = {0};
	std::string pattern;
	int r = GetTopUrl(top, pattern);
	if(r)
		return r;

	for(int page=1; page<=10; page++)
	{
		char* result;
		sprintf(uri, pattern.c_str(), page);
		for(int i=0; i<10; i++)
		{
			int r = WebToXml(uri, NULL, "E:\\app\\web\\zongheng\\top.xml", &result);
			if(r < 0)
			{
				printf("get %s error: %d\n", uri, r);
				system_sleep(10);
				continue;
			}

			printf("[%s] page[%d] ok\n", g_name, page);
			break;
		}

		r = ParseXml(result, callback, param);
		free(result);

		if(r > 0)
			return r; // return by callback
	}

	return 0;
}

static int Register()
{
	static book_site_t site;
	site.id = 2;
	site.name = g_name;
	site.spider = Spider;
	return book_site_register(&site);
}

static int dummy = Register();
