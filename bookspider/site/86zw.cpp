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
	char name[256] = {0};
	char request[1024] = {0};

	url_encode_utf8(book, name, sizeof(name));
	sprintf(request, "searchkey=%s&searchtype=articlename", name);

	char* result;
	const char* uri = "http://www.86zw.org/modules/article/search.php";
	int r = WebToXml(uri, request, "E:\\app\\web\\86zw.org\\search.xml", &result);
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

	const char* p = strrchr(bookUri, '.');
	strncpy(indexUri, bookUri, p-bookUri);
	strcpy(indexUri+(p-bookUri), "/index.html");
	return 0;
}

static const char* g_name = "86zw";
static const char* g_index = "E:\\app\\web\\86zw.org\\index.xml";
static int Register()
{
	static book_spider_t spider;
	spider.name = g_name;
	spider.index = g_index;
	spider.interval = 10;
	spider.search = SearchBook;
	return book_spider_register(&spider);
}

//static int dummy = Register();
