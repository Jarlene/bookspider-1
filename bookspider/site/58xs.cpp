#include "bookspider.h"
#include "bookspidertools.h"
#include "cstringext.h"
#include "netmanager.h"

static int Compare(const char* name, const char* author, const char* uri, const char* chapter, va_list args)
{
	const char* vname = va_arg(args, const char*);
	const char* vauthor = va_arg(args, const char*);
	char* vbook = va_arg(args, char*);
	char* vindex = va_arg(args, char*);

	if(strieq(name, vname) && strieq(author, vauthor))
	{
		strcpy(vbook, uri);
		const char* path = strrchr(chapter, '/');
		if(path)
		{
			strncpy(vindex, chapter, path-chapter);
			vindex[path-chapter] = 0;
			strcat(vindex, "/index.html");
		}
		return 1; // find it
	}

	return 0;
}

static int SearchBook(const char* book, const char* author, char* bookUri, char* indexUri)
{
	char name[256] = {0};
	char request[1024] = {0};

	url_encode_utf8(book, name, sizeof(name));
	snprintf(request, sizeof(request)-1, "searchtype=articlename&searchkey=%s&Submit=+%%CB%%D1+%%CB%%F7+", name);

	char* result;
	const char* uri = "http://book.58xs.com/modules/article/search.php";
	int r = WebToXml(uri, request, "E:\\app\\web\\58xs.com\\search.xml", &result);
	if(r)
	{
		assert(r < 0);
		return r;
	}

	// book: http://book.58xs.com/191859.html
	// chapter: http://book.58xs.com/html/191/191859/18110147.html
	r = parse_search_result(result, Compare, book, author, bookUri, indexUri);
	free(result);
	if(r < 0)
		return -1;
	return 0;
}

static const char* g_name = "58xs";
static const char* g_index = "E:\\app\\web\\58xs.com\\index.xml";
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
