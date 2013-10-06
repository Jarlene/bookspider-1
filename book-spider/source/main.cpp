#include "cstringext.h"
#include "sys/sock.h"
#include "time64.h"
#include "error.h"
#include "utf8codec.h"
#include "bookmanager.h"
#include <stdio.h>

#include "58xs.h"
#include "86zw.h"
#include "luoqiu.h"

IBookSpider* CreateSpider(const char* site)
{
	if(strieq(site, "58xs")) return new C58XS();
	else if(strieq(site, "86zw")) return new C86ZW();
	else if(strieq(site, "luoqiu")) return new CLuoQiu();

	return NULL;
}

static int OnListBook(void* param, const char* book, const char* author, const char* uri, const char* chapter, const char* datetime)
{
	BookManager* bookmgr = BookManager::FetchBookManager();
	if(!bookmgr)
	{
		printf("fetch book manager error.\n");
		return ERROR_PARAM;
	}

	IBookSpider* spider = (IBookSpider*)param;
	int r = bookmgr->SetChapter(spider->GetName(), book, author, uri, chapter, datetime);
	if(r < 0)
	{
		printf("OnListBook[%s] set chapter[%s/%s/%s/%s] error: %d\n", 
			spider->GetName(), 
			(const char*)UTF8Decode(book), 
			(const char*)UTF8Decode(author), 
			(const char*)UTF8Decode(chapter), 
			(const char*)UTF8Decode(datetime),
			r);
		return r;
	}

	return 0;
}

static int BookList(const char* site)
{
	IBookSpider* spider = CreateSpider(site);
	if(NULL == spider)
	{
		printf("don't find %s\n", site);
		return ERROR_NOTFOUND;
	}

	return spider->List(OnListBook, spider);
}

static char g_last_check_datetime[20] = {0};
static int OnCheckBook(void* param, const char* book, const char* author, const char* uri, const char* chapter, const char* datetime)
{
	BookManager* bookmgr = BookManager::FetchBookManager();
	if(!bookmgr)
	{
		printf("fetch book manager error.\n");
		return ERROR_PARAM;
	}

	IBookSpider* spider = (IBookSpider*)param;
	int r = bookmgr->SetChapter(spider->GetName(), book, author, uri, chapter, datetime);
	if(r < 0)
	{
		printf("OnCheckBook[%s] set chapter[%s/%s/%s/%s] error: %d\n", 
			spider->GetName(), 
			(const char*)UTF8Decode(book), 
			(const char*)UTF8Decode(author), 
			(const char*)UTF8Decode(chapter), 
			(const char*)UTF8Decode(datetime),
			r);
		return r;
	}

	// last check datetime
	if(strcmp(datetime, g_last_check_datetime) < 0)
		return 1;

	return 0;
}

static int BookUpdate(const char* site)
{
	IBookSpider* spider = CreateSpider(site);
	if(NULL == spider)
	{
		printf("don't find %s\n", site);
		return ERROR_NOTFOUND;
	}

	BookManager* bookmgr = BookManager::FetchBookManager();
	if(!bookmgr)
	{
		printf("fetch book manager error.\n");
		return ERROR_PARAM;
	}

	bookmgr->GetLastCheckDatetime(spider->GetName(), g_last_check_datetime);

	return spider->Check(OnCheckBook, spider);
}

int main(int argc, char* argv[])
{
	socket_init();

	for(int i=1; i<argc; i++)
	{
		if(streq(argv[i], "--list") && i+1<argc)
		{
			BookList(argv[++i]);
		}
		else if(streq(argv[i], "--update") && i+1<argc)
		{
			BookUpdate(argv[++i]);
		}
		else
		{
			printf("Book-Spider [--list site | --update site]\n");
			break;
		}
	}

	socket_cleanup();
	return 0;
}
