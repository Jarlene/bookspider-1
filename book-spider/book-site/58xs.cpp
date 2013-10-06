#include "58xs.h"
#include "cstringext.h"
#include "urlcodec.h"
#include "utf8codec.h"
#include <string>

int C58XS::Search(const char* book, const char* author, char *bookUri)
{
	char name[256] = {0};
	char request[256] = {0};

	int r = url_encode(UTF8Decode(book), -1, name, sizeof(name));
	snprintf(request, sizeof(request)-1, "searchtype=articlename&searchkey=%s&Submit=+%%CB%%D1+%%CB%%F7+", name);

	const char* uri = "http://book.58xs.com/modules/article/search.php";
	r = SearchBook(this, uri, request, book, author, name);
	if(0 == r)
	{
		// book: http://book.58xs.com/191859.html
		// index: http://book.58xs.com/html/191/191859/index.html
		int bookid = 0;
		if(1 != sscanf(name, "http://book.58xs.com/%d.html", &bookid))
		{
			printf("C58XS::Search get bookid error: %s\n", name);
			return -1;
		}

		sprintf(bookUri, "http://book.58xs.com/html/%d/%d/index.html", bookid/1000, bookid);
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

int C58XS::Check(OnBook callback, void* param)
{
	int r = 0;
	char uri[256] = {0};
	HandleParam p = {callback, param};
	for(int page=1; 0 == r; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://www.58xs.com/xiaoshuotoplastupdate/0/0/%d.html", page);

		r = ListBook(this, uri, NULL, OnHandle, &p);
	}

	return r;
}

int C58XS::List(OnBook callback, void* param)
{
	int r = 0;
	char uri[256] = {0};
	HandleParam p = {callback, param};
	for(int page=1; 0 == r; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://www.58xs.com/xiaoshuotopallvisit/0/0/%d.html", page);

		r = ListBook(this, uri, NULL, OnHandle, &p);
	}

	return r;
}
