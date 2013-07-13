#include "bookspider.h"
#include "bookspidertools.h"
#include "cstringext.h"
#include "netmanager.h"
#include "error.h"

static int Compare(const char* name, const char* author, const char* uri, const char* /*chapter*/, va_list args)
{
	const char* vname = va_arg(args, const char*);
	const char* vauthor = va_arg(args, const char*);
	char* vuri = va_arg(args, char*);

	if(strieq(name, vname) && strieq(author, vauthor))
	{
		strcpy(vuri, uri);
		return 1; // find it
	}

	return 0;
}

static int SearchBook(const char* book, const char* author, char* bookUri, char* indexUri)
{
	char uri[256] = {0};
	char name[256] = {0};

	url_encode_utf8(book, name, sizeof(name));
	snprintf(uri, sizeof(uri)-1, "http://www.luoqiu.com/modules/article/search.php?searchkey=%s&searchtype=articlename", name);

	char* result;
	int r = WebToXml(uri, NULL, "E:\\app\\web\\luoqiu.com\\search.xml", &result);
	if(0 == r)
	{
		r = parse_search_result(result, Compare, book, author, bookUri);
		free(result);
		if(r < 0)
			return r;
	}
	else if(ERROR_HTTP_REDIRECT == r)
	{
		strcpy(bookUri, result);
		free(result);
	}
	else
	{
		assert(r < 0);
		return r;
	}
	
	// book: http://www.luoqiu.com/book/69/69851.html
	// chapter: http://www.luoqiu.com/html/69/69851/9887547.html
	char* p = strrchr(bookUri, '.');
	strncpy(indexUri, bookUri, p-bookUri);
	strcpy(indexUri+(p-bookUri), "/index.html");
	p = strstr(indexUri, "/book/");
	memcpy(p, "/html/", 6);
	return 0;
}

static const char* g_name = "luoqiu";
static const char* g_index = "E:\\app\\web\\luoqiu.com\\index.xml";
static int Register()
{
	static book_spider_t spider;
	spider.name = g_name;
	spider.index = g_index;
	spider.interval = 10;
	spider.search = SearchBook;
	return book_spider_register(&spider);
}

static int dummy = Register();
