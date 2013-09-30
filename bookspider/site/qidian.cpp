#include "booksite.h"
#include <string>
#include "sys/system.h"
#include "XMLParser.h"
#include "netmanager.h"
#include "utf8.h"

static const char* g_name = "qidian";

struct TopUrls
{
	int type;
	std::string url;
};

static TopUrls g_urls[] = { 
	{ ETT_ALL_VIEW, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=13&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_ALL_MARK, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=9&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_ALL_VOTE, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=3&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_MONTH_VIEW, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=12&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_MONTH_VOTE, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=4&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_WEEK_VIEW, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=11&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_WEEK_VOTE, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=8&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
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
		book.siteid = 1;
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

static int Spider(int top, book_site_spider_fcb callback, void* param)
{
	char uri[256] = {0};
	std::string pattern;
	int r = GetTopUrl(top, pattern);
	if(r)
		return r;

	for(int page=1; page<=5; page++)
	{
		char* result;
		sprintf(uri, pattern.c_str(), page);
		for(int i=0; i<10; i++)
		{
			int r = WebToXml(uri, NULL, "E:\\app\\web\\qidian\\top.xml", &result);
			if(r < 0)
			{
				printf("qidian uri %s get page error: %d\n", uri, r);
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
	site.id = 1;
	site.name = g_name;
	site.spider = Spider;
	return book_site_register(&site);
}

//static int dummy = Register();
