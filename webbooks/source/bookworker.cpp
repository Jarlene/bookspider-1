#include "sys/process.h"
#include "sys/sync.h"
#include "sys/sync.hpp"
#include "cstringext.h"
#include "booksite.h"
#include "bookmanager.h"
#include <stdio.h>
#include <assert.h>
#include <vector>
#include "booktop.h"
#include "jsonhelper.h"
#include "time64.h"

struct BookSpiderParam
{
	const char* sitename;
	jsonarray json;
	int num;
	int total;
};

struct BookStatistic
{
	long newbooks;
	long updatebooks;
};

static BookStatistic g_bookstatistic;

static std::map<int, BookTop> g_booktop;

extern int g_param_bookall;

static int UpdateBookTop(int top, const char* sitename, const char* book, const char* author, int vote)
{
	static ThreadLocker s_locker;

	int bookid = 0;
	BookManager* bookmgr = BookManager::FetchBookManager();
	if(bookmgr->GetBookId(book, author, bookid))
		return -1; // book don't exist

	AutoThreadLocker locker(s_locker);
	if(0 == g_booktop.size())
	{
		g_booktop.insert(std::make_pair(ETT_ALL_VOTE, BookTop()));
		g_booktop.insert(std::make_pair(ETT_ALL_MARK, BookTop()));
		g_booktop.insert(std::make_pair(ETT_ALL_VOTE, BookTop()));
		g_booktop.insert(std::make_pair(ETT_MONTH_VOTE, BookTop()));
		g_booktop.insert(std::make_pair(ETT_MONTH_MARK, BookTop()));
		g_booktop.insert(std::make_pair(ETT_MONTH_VIEW, BookTop()));
		g_booktop.insert(std::make_pair(ETT_WEEK_VOTE, BookTop()));
		g_booktop.insert(std::make_pair(ETT_WEEK_MARK, BookTop()));
		g_booktop.insert(std::make_pair(ETT_WEEK_VIEW, BookTop()));
	}

	std::map<int, BookTop>::iterator it = g_booktop.find(top);
	if(it == g_booktop.end())
		return -1;

	it->second.AddBook(sitename, bookid, vote);
	return 0;
}

static int GetBookNum(const char* sitename, int top)
{
	switch(top)
	{
	case ETT_ALL_VOTE:
	case ETT_ALL_MARK:
	case ETT_ALL_VIEW:
		if(strieq("qidian", sitename))
			return 12000;
		else if(strieq("zongheng", sitename))
			return 5000;
		else
			return 2000;

	case ETT_MONTH_VOTE:
	case ETT_MONTH_MARK:
	case ETT_MONTH_VIEW:
		return 500;

	case ETT_WEEK_VOTE:
	case ETT_WEEK_MARK:
	case ETT_WEEK_VIEW:
		return 200;

	default:
		assert(false);
		return 200;
	}
}

static int AddBook(const char* sitename, const book_t* book)
{
	BookManager* bookmgr = BookManager::FetchBookManager();

	BookManager::Book item;
	memcpy(&item, book, sizeof(BookManager::Book));
	
	int r = bookmgr->AddBook(item);
	if(r < 0)
	{
		return r;
	}
	else if(0 == r)
	{
		// add book
		InterlockedIncrement(&g_bookstatistic.newbooks);
	}
	else if(1 == r)
	{
		// update book
		InterlockedIncrement(&g_bookstatistic.updatebooks);
	}
	else
	{
		assert(false);
		return -1;
	}
	return 0;
}

static int BookSpider(void* param, const book_t* book)
{
	BookSpiderParam* p = (BookSpiderParam*)param;

	jsonobject json;
	json.add("name", book->name);
	json.add("author", book->author);
	json.add("category", book->category);
	json.add("count", book->vote);
	p->json.add(json);
	//int r = AddBook(p->sitename, book);
	//if(r < 0)
	//{
	//	printf("%s add book %s err: %d\n", p->sitename, book->name, r);
	//	return 0;
	//}

	//// top 500
	//if(p->num <= 500)
	//	UpdateBookTop(p->top, p->sitename, book->name, book->author, book->vote);
	return 0;
}

static int BookSiteWorker(void* param)
{
	const book_site_t* site = book_site_get((int)param);

	//if(g_param_bookall)
	//{
	//	p.num = 0;
	//	p.top = ETT_ALL_VOTE;
	//	p.total = GetBookNum(site->name, ETT_ALL_VOTE);
	//	site->spider(ETT_ALL_VOTE, BookSpider, &p);
	//}

	ETopType types[] = {ETT_MONTH_VIEW,ETT_MONTH_VOTE};
	for(int i=0; i<sizeof(types)/sizeof(types[0]); i++)
	{
		BookSpiderParam p;
		p.sitename = site->name;
		p.num = 0;
		p.total = GetBookNum(site->name, types[i]);
		site->spider(types[i], BookSpider, &p);

		char date[16] = {0};
		time64_format(time64_now(), "%04Y-%02M-%02D", date);
		
		char filename[128]= {0};
		snprintf(filename, sizeof(filename), "%s-%d-%s", site->name, types[i], date);

		FILE* fp = fopen(filename, "w");
		std::string json = p.json.json();
		fwrite(json.c_str(), json.length(), 1, fp);
		fclose(fp);
	}

	//p.num = 0;
	//p.top = ETT_WEEK_VOTE;
	//p.total = GetBookNum(site->name, ETT_WEEK_VOTE);
	//site->spider(ETT_WEEK_VOTE, BookSpider, &p);
	return 0;
}

int BookWorker()
{
	// start worker thread
	std::vector<thread_t> threads;
	for(int i=0; i<book_site_count(); i++)
	{
		thread_t thread;
		thread_create(&thread, BookSiteWorker, (void*)i);
		threads.push_back(thread);
	}

	// wait for all thread exit
	for(size_t i=0; i<threads.size(); i++)
	{
		thread_destroy(threads[i]);
	}

	printf("BookWorker: add book %ld, update book %ld\n", 
		g_bookstatistic.newbooks, g_bookstatistic.updatebooks);

	// dump book top to file
	//if(g_param_bookall)
	//	g_booktop.find(ETT_ALL_VOTE)->second.Save("topallvote.xml");
	//g_booktop.find(ETT_MONTH_VOTE)->second.Save("topmonthvote.xml");
	//g_booktop.find(ETT_WEEK_VOTE)->second.Save("topweekvote.xml");
	return 0;
}
