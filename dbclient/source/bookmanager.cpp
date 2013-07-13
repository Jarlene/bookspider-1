#include "bookmanager.h"
#include "dbclient.h"
#include <stdio.h>
#include <assert.h>
#include "sys/sync.hpp"
#include "cstringext.h"
extern "C"
{
#include "systimeconfig.h"
}

static ThreadLocker g_locker;

BookManager* BookManager::FetchBookManager()
{
	static BookManager s_bookmgr;
	return &s_bookmgr;
}

BookManager::BookManager()
{
	db_init();
	m_db = db_connect("192.168.133.130", 3306, "books", "root", "");
}

BookManager::~BookManager()
{
	db_disconnect(m_db);
	db_fini();
}

int BookManager::AddBook(const Book& book)
{
	char sql[2*1024] = {0};
	AutoThreadLocker locker(g_locker);

	int bookid = 0;
	if(GetBookId(book.name, book.author, bookid))
	{
		snprintf(sql, sizeof(sql)-1,
			"update books set uri='%s', datetime='%s', chapter='%s', mid=%d, vote=%d where bid=%d", 
			book.uri, book.datetime, book.chapter, db_mid(), bookid);
		int r = db_update(m_db, sql);
		return 0==r ? 1 : r;
	}
	else
	{
		assert(0 == strchr(book.name, '\''));
		assert(0 == strchr(book.author, '\''));
		assert(0 == strchr(book.uri, '\''));
		assert(0 == strchr(book.datetime, '\''));
		assert(0 == strchr(book.chapter, '\''));
		snprintf(sql, sizeof(sql)-1, 
			"insert into books (name, author, uri, datetime, mid, vote, category, chapter, siteid) values ('%s', '%s', '%s', '%s', %d, %d, '%s', '%s', %d)",
			book.name, book.author, book.uri, book.datetime, db_mid(), book.vote, book.category, book.chapter, book.siteid);
		int r = db_insert(m_db, sql);
		return 0==r ? 0 : r;
	}
}

int BookManager::QueryBook(int mid, int from, int count, std::vector<std::pair<int, Book> >& books) const
{
	char sql[1024] = {0};
	AutoThreadLocker locker(g_locker);

	from = from<0?0:from;
	count = count==0?100:count;
	sprintf(sql, "select * from books where mid>%d limit %d,%d", mid, from, count);

	DBQueryResult result;
	int r = db_query(m_db, sql, result);
	if(r)
		return r;

	while(!result.FetchRow())
	{
		int bid;
		Book book;
		r = result.GetValue("bid", bid);
		r = result.GetValue("name", book.name, sizeof(book.name));
		r = result.GetValue("author", book.author, sizeof(book.author));
		r = result.GetValue("uri", book.uri, sizeof(book.uri));
		r = result.GetValue("datetime", book.datetime, sizeof(book.datetime));
		r = result.GetValue("category", book.category, sizeof(book.category));
		r = result.GetValue("chapter", book.chapter, sizeof(book.chapter));
		r = result.GetValue("siteid", book.siteid);
		r = result.GetValue("vote", book.vote);

		books.push_back(std::make_pair(bid, book));
	}
	
	return 0;
}

int BookManager::GetBookChapter(int bookid, char chapter1[100], char chapter2[100], char chapter3[100])
{
	char sql[100] = {0};
	AutoThreadLocker locker(g_locker);

	sprintf(sql, "select chapter, chapter2, chapter3 from books where bid=%d", bookid);
	DBQueryResult result;
	int r = db_query(m_db, sql, result);
	if(0 == r)
	{
		r = result.FetchRow();
		if(0 == r)
		{
			r = result.GetValue(0, chapter3, 100);
			r = result.GetValue(1, chapter2, 100);
			r = result.GetValue(2, chapter1, 100);
		}
	}
	return r;
}

int BookManager::SetBookChapter(int bookid, const char* chapter1, const char* chapter2, const char* chapter3)
{
	char sql[2*1024] = {0};
	AutoThreadLocker locker(g_locker);

	assert(0 == strchr(chapter1, '\''));
	assert(0 == strchr(chapter2, '\''));
	assert(0 == strchr(chapter3, '\''));
	snprintf(sql, sizeof(sql)-1,
		"update books set chapter='%s', chapter2='%s', chapter3='%s', mid=%d where bid=%d", 
		chapter3, chapter2, chapter1, db_mid(), bookid);
	return db_update(m_db, sql);
}

bool BookManager::CheckBookSite(int bookid, const char* site) const
{
	char sql[100] = {0};
	AutoThreadLocker locker(g_locker);

	snprintf(sql, sizeof(sql)-1, "select bid from %s where bid=%d", site, bookid);

	DBQueryResult result;
	if(db_query(m_db, sql, result))
		return false;
	return 0==result.FetchRow();
}

int BookManager::AddBookSite(int bookid, const char* site, const char* bookuri, const char* indexuri)
{
	char sql[2*1024] = {0};
	AutoThreadLocker locker(g_locker);

	assert(0 == strchr(bookuri, '\''));
	assert(0 == strchr(indexuri, '\''));
	snprintf(sql, sizeof(sql)-1, 
		"insert into %s (bid, bookuri, indexuri, mid) values (%d, '%s', '%s', %d)",
		site, bookid, bookuri, indexuri, db_mid());
	return db_insert(m_db, sql);
}

//int BookManager::ClearBookSite(int bookid)
//{
//	sprintf(sql, "delete from booksite where bid=%d", bookid);
//	return db_delete(m_db, sql);
//}

int BookManager::QueryBookFromSite(const char* site, int from, int count, std::vector<BookSite>& sites)
{
	char sql[100] = {0};
	AutoThreadLocker locker(g_locker);

	from = from<0?0:from;
	count = count==0?100:count;
	sprintf(sql, "select * from %s limit %d, %d", site, from, count);

	DBQueryResult result;
	int r = db_query(m_db, sql, result);
	if(r)
		return r;

	while(!result.FetchRow())
	{
		BookSite item;
		r = result.GetValue("bid", item.bookid);
		r = result.GetValue("indexuri", item.uri, sizeof(item.uri));
		sites.push_back(item);
	}

	return 0;
}

int BookManager::SetBookSiteChapter(const char* site, int bookid, const char* chapter)
{
	char sql[2*1024] = {0};
	AutoThreadLocker locker(g_locker);

	assert(0 == strchr(site, '\''));
	assert(0 == strchr(chapter, '\''));
	snprintf(sql, sizeof(sql)-1,
		"update %s set chapter='%s', mid=%d where bid=%d", 
		site, chapter, db_mid(), bookid);
	return db_update(m_db, sql);
}

int BookManager::GetBookId(const char* name, const char* author, int& bookid) const
{
	char sql[2*1024] = {0};
	AutoThreadLocker locker(g_locker);

	assert(0 == strchr(name, '\''));
	assert(0 == strchr(author, '\''));
	snprintf(sql, sizeof(sql)-1, "select bid from books where name='%s' and author='%s'", name, author);

	DBQueryResult result;
	if(db_query(m_db, sql, result))
		return false;
	if(result.FetchRow())
		return false;
	return 0==result.GetValue(0, bookid);
}

int BookManager::GetBookInfo(const char* name, const char* author, Book& book) const
{
	char sql[2*1024] = {0};
	AutoThreadLocker locker(g_locker);

	assert(0 == strchr(name, '\''));
	assert(0 == strchr(author, '\''));
	snprintf(sql, sizeof(sql)-1, "select * from books where name='%s' and author='%s'", name, author);

	DBQueryResult result;
	int r = db_query(m_db, sql, result);
	if(!r)
	{
		r = result.FetchRow();
		if(!r)
		{
			r = result.GetValue("name", book.name, sizeof(book.name));
			r = result.GetValue("author", book.author, sizeof(book.author));
			r = result.GetValue("uri", book.uri, sizeof(book.uri));
			r = result.GetValue("datetime", book.datetime, sizeof(book.datetime));
			r = result.GetValue("category", book.category, sizeof(book.category));
			r = result.GetValue("chapter", book.chapter, sizeof(book.chapter));
			r = result.GetValue("siteid", book.siteid);
			r = result.GetValue("vote", book.vote);
		}
	}
	return r;
}

int BookManager::GetBookInfo(int bookid, Book& book) const
{
	char sql[100] = {0};
	AutoThreadLocker locker(g_locker);

	sprintf(sql, "select * from books where bid='%d'", bookid);

	DBQueryResult result;
	int r = db_query(m_db, sql, result);
	if(!r)
	{
		r = result.FetchRow();
		if(!r)
		{
			r = result.GetValue("name", book.name, sizeof(book.name));
			r = result.GetValue("author", book.author, sizeof(book.author));
			r = result.GetValue("uri", book.uri, sizeof(book.uri));
			r = result.GetValue("datetime", book.datetime, sizeof(book.datetime));
			r = result.GetValue("category", book.category, sizeof(book.category));
			r = result.GetValue("chapter", book.chapter, sizeof(book.chapter));
			r = result.GetValue("siteid", book.siteid);
			r = result.GetValue("vote", book.vote);
		}
	}
	return r;
}

int BookManager::db_mid()
{
	AutoThreadLocker locker(g_locker);

	static int s_id = 0;
	if(0==s_id)
	{
		char sql[100] = {0};
		char time[24] = {0};
		system_gettime(time);
		snprintf(sql, sizeof(sql)-1, "insert into booklog (datetime) values ('%s')", time);
		if(0 == db_insert(m_db, sql))
		{
			DBQueryResult result;
			if(0 == db_query(m_db, "select max(id) from booklog", result))
			{
				if(0 == result.FetchRow())
					result.GetValue(0, s_id);
			}
		}
	}

	return s_id;
}
