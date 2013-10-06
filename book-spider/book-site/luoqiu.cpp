#include "luoqiu.h"
#include "cstringext.h"
#include "urlcodec.h"
#include "utf8codec.h"
#include <string>

int CLuoQiu::Search(const char* book, const char* author, char *bookUri)
{
	char uri[256] = {0};
	char name[256] = {0};

	int r = url_encode(UTF8Decode(book), -1, name, sizeof(name));
	snprintf(uri, sizeof(uri)-1, "http://www.luoqiu.com/modules/article/search.php?searchkey=%s&searchtype=articlename", name);
	r = SearchBook(this, uri, NULL, book, author, name);
	if(0 == r)
	{
		// book: http://www.luoqiu.com/book/69/69851.html
		// index: http://www.luoqiu.com/html/69/69851/index.html
		char* p = strrchr(name, '.');
		if(p)
		{
			*p = 0;
			sprintf(bookUri, "%s/index.html", name);
		}
	}
	return r;
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

int CLuoQiu::Check(OnBook callback, void* param)
{
	int r = 0;
	char uri[256] = {0};
	HandleParam p = {callback, param};
	for(int page=1; 0 == r; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://www.luoqiu.com/gengxin/%d.html", page);

		r = ListBook(this, uri, NULL, OnHandle, &p);
	}

	return r;
}

int CLuoQiu::List(OnBook callback, void* param)
{
	int r = 0;
	char uri[256] = {0};
	HandleParam p = {callback, param};
	for(int page=1; 0 == r; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://www.luoqiu.com/gengxin/%d.html", page);

		r = ListBook(this, uri, NULL, OnHandle, &p);
	}

	return r;
}
