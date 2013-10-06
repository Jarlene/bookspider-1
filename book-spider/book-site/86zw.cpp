#include "86zw.h"
#include "cstringext.h"
#include "urlcodec.h"
#include "utf8codec.h"
#include <string>

int C86ZW::Search(const char* book, const char* author, char *bookUri)
{
	char name[256] = {0};
	char request[256] = {0};

	int r = url_encode(UTF8Decode(book), -1, name, sizeof(name));
	sprintf(request, "searchkey=%s&searchtype=articlename", name);

	const char* uri = "http://www.86zw.org/modules/article/search.php";
	r = SearchBook(this, uri, request, book, author, name);
	if(0 == r)
	{
		// book: http://www.86zw.org/html/2/2355.html
		// index: http://www.86zw.org/html/2/2355/index.html
		char* p = strrchr(name, '.');
		if(p)
		{
			*p = 0;
			sprintf(bookUri, "%s/index.html", name);
		}
	}
	return 0;
}

struct HandleParam
{
	IBookSpider::OnBook callback;
	void* param;
};

static int OnHandle(void* param, const char* book, const char* author, const char* uri, const char* chapter, const char* datetime)
{
	HandleParam* o = (HandleParam*)param;
	const char* p = strrchr(uri, '/');
	if(!p)
	{
		printf("%s:%s invalid chapter uri: %s.\n", __FILE__, __LINE__, uri);
		return 0;
	}

	std::string ch(uri, p-uri);
	ch += "/index.html";
	return o->callback(o->param, book, author, ch.c_str(), chapter, datetime);
}

int C86ZW::Check(OnBook callback, void* param)
{
	int r = 0;
	char uri[256] = {0};
	HandleParam p = {callback, param};
	for(int page=1; 0 == r; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://www.86zw.org/booktop/lastupdate_%d.html", page);

		r = ListBook(this, uri, NULL, OnHandle, &p);
	}

	return r;
}

int C86ZW::List(OnBook callback, void* param)
{
	int r = 0;
	char uri[256] = {0};
	HandleParam p = {callback, param};
	for(int page=1; 0 == r; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://www.86zw.org/booktop/allvisit_%d.html", page);

		r = ListBook(this, uri, NULL, OnHandle, &p);
	}

	return r;
}
