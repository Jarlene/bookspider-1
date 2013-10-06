#include "bookmanager.h"
#include "dbclient.h"
#include "cstringext.h"
#include "sys/sync.hpp"
#include "time64.h"
#include "error.h"
#include <stdio.h>
#include <assert.h>

static ThreadLocker g_locker;

BookManager* BookManager::FetchBookManager()
{
	static BookManager s_bookmgr;
	return &s_bookmgr;
}

BookManager::BookManager()
{
	db_init();
	m_db = db_connect("192.168.133.135", 3306, "books", "root", "");
}

BookManager::~BookManager()
{
	db_disconnect(m_db);
	db_fini();
}

int BookManager::AddBook(const Book& book)
{
	assert(0 != book.id);
	assert(0 == strchr(book.name, '\''));
	assert(0 == strchr(book.author, '\''));
	assert(0 == strchr(book.uri, '\''));
	assert(0 == strchr(book.datetime, '\''));
	assert(0 == strchr(book.chapter, '\''));

	int r = 0;
	char sql[2*1024] = {0};

	AutoThreadLocker locker(g_locker);
	if(HaveBook(book.id))
	{
		snprintf(sql, sizeof(sql)-1, 
			"update books set category='%s', uri='%s', chapter='%s', vote=%d, datetime='%s' where bid=%d", 
			book.category, book.uri, book.chapter, book.vote, book.datetime, book.id);
		r = db_update(m_db, sql);
		return r;
	}
	else
	{
		snprintf(sql, sizeof(sql)-1, 
			"insert into books (bid, name, author, category, uri, chapter, vote, datetime) values (%d, '%s', '%s', '%s', '%s', '%s', %d, '%s')",
			book.id, book.name, book.author, book.category, book.uri, book.chapter, book.vote, book.datetime);
		r = db_insert(m_db, sql);
		return r >= 0 ? 0 : r;
	}
}

int BookManager::QueryBook(int mid, int from, int count, Books& books) const
{
	char sql[1024] = {0};
	from = from<0?0:from;
	count = count==0?100:count;
	sprintf(sql, "select * from books where mid>%d limit %d,%d", mid, from, count);

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(m_db, sql, result);
	if(0 != r)
		return r;

	Book book;
	while(0 == result.FetchRow())
	{
		memset(&book, 0, sizeof(book));
		r = result.GetValue("bid", book.id);
		r = result.GetValue("name", book.name, sizeof(book.name));
		r = result.GetValue("author", book.author, sizeof(book.author));
		r = result.GetValue("uri", book.uri, sizeof(book.uri));
		r = result.GetValue("datetime", book.datetime, sizeof(book.datetime));
		r = result.GetValue("category", book.category, sizeof(book.category));
		r = result.GetValue("chapter", book.chapter, sizeof(book.chapter));
		r = result.GetValue("vote", book.vote);

		books.push_back(book);
	}

	return 0;
}

int BookManager::QueryBookFromSite(const char* site, int from, int count, std::vector<BookSite>& sites)
{
	char sql[100] = {0};
	from = from<0?0:from;
	count = count==0?100:count;
	sprintf(sql, "select * from %s limit %d, %d", site, from, count);

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(m_db, sql, result);
	if(0 != r)
		return r;

	while(0 == result.FetchRow())
	{
		BookSite item;
		memset(&item, 0, sizeof(item));
		r = result.GetValue("bid", item.bookid);
		r = result.GetValue("indexuri", item.uri, sizeof(item.uri));
		sites.push_back(item);
	}

	return 0;
}

int BookManager::SetBookSiteChapter(const char* site, int bookid, const char* chapter)
{
	assert(0 == strchr(site, '\''));
	assert(0 == strchr(chapter, '\''));

	char sql[2*1024] = {0};
	snprintf(sql, sizeof(sql)-1,
		"update %s set chapter='%s', mid=%d where bid=%d", 
		site, chapter, db_mid(), bookid);

	AutoThreadLocker locker(g_locker);
	int r = db_update(m_db, sql);
	return r >= 0 ? 0 : r;
}

int BookManager::GetBookId(const char* name, const char* author, int& bookid) const
{
	assert(0 == strchr(name, '\''));
	assert(0 == strchr(author, '\''));

	char sql[2*1024] = {0};
	snprintf(sql, sizeof(sql)-1, "select bid from books where name='%s' and author='%s'", name, author);

	AutoThreadLocker locker(g_locker);
	return db_query_int(m_db, sql, &bookid);
}

bool BookManager::HaveBook(int bookid)
{
	char sql[1024] = {0};
	snprintf(sql, sizeof(sql)-1, "select bid from books where bid=%d", bookid);

	AutoThreadLocker locker(g_locker);
	return 0==db_query_int(m_db, sql, &bookid);
}

int BookManager::GetBookInfo(const char* name, const char* author, Book& book) const
{
	assert(0 == strchr(name, '\''));
	assert(0 == strchr(author, '\''));

	char sql[2*1024] = {0};
	snprintf(sql, sizeof(sql)-1, "select * from books where name='%s' and author='%s'", name, author);

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(m_db, sql, result);
	if(0 == r)
	{
		r = result.FetchRow();
		if(0 == r)
		{
			r = result.GetValue("name", book.name, sizeof(book.name));
			r = result.GetValue("author", book.author, sizeof(book.author));
			r = result.GetValue("uri", book.uri, sizeof(book.uri));
			r = result.GetValue("datetime", book.datetime, sizeof(book.datetime));
			r = result.GetValue("category", book.category, sizeof(book.category));
			r = result.GetValue("chapter", book.chapter, sizeof(book.chapter));
			r = result.GetValue("vote", book.vote);
		}
	}
	return r;
}

int BookManager::GetBookInfo(int bookid, Book& book) const
{
	char sql[100] = {0};
	sprintf(sql, "select * from books where bid='%d'", bookid);

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(m_db, sql, result);
	if(0 == r)
	{
		r = result.FetchRow();
		if(0 == r)
		{
			r = result.GetValue("name", book.name, sizeof(book.name));
			r = result.GetValue("author", book.author, sizeof(book.author));
			r = result.GetValue("uri", book.uri, sizeof(book.uri));
			r = result.GetValue("datetime", book.datetime, sizeof(book.datetime));
			r = result.GetValue("category", book.category, sizeof(book.category));
			r = result.GetValue("chapter", book.chapter, sizeof(book.chapter));
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
		snprintf(sql, sizeof(sql)-1, "insert into booklog (datetime) values (now())");
		if(db_insert(m_db, sql) > 0)
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

int BookManager::SetTopBooks(EBookTop top, const Books& books)
{
	char month[8] = {0};
	time64_format(time64_now(), "%04Y%02M", month);

	char sql[128] = {0};
	std::string sqlvalue;
	Books::const_iterator it;
	for(it = books.begin(); it != books.end(); ++it)
	{
		const Book& book = *it;
		assert(0 != book.id);

		int r = 0;
		if(!HaveBook(book.id))
		{
			// add book if don't exist
			r = AddBook(book);
		}

		if(0 != r)
			return r;

		sprintf(sql, "(%d, %d, '%s')", book.id, book.vote, month);
		if(!sqlvalue.empty())
			sqlvalue += ", ";
		sqlvalue += sql;
	}

	const char *tables[] = { 
		"top_all_view", "top_all_mark", "top_all_vote", 
		"top_month_view", "top_month_mark", "top_month_vote", 
		"top_week_view", "top_week_mark", "top_week_vote" 
	};
	assert(top > 0 && top < sizeof(tables)/sizeof(tables[0]));
	sprintf(sql, "insert into %s (bid, n, month) values ", tables[top-1]);
	sqlvalue.insert(0, sql); // merge sql statement

	assert(books.size() > 0);
	//sprintf(sql, "delete from %s where month='%s' and bid in (select bid from books where site=%d)", tables[top-1], month, books.begin()->siteid);
	sprintf(sql, "delete from %s where month='%s' and (bid DIV %d)=%d", tables[top-1], month, BOOK_ID, books.begin()->id/BOOK_ID);

	AutoThreadLocker locker(g_locker);
	int r = db_delete(m_db, sql); // clear
	if(r < 0)
		return r;

	r = db_insert(m_db, sqlvalue.c_str()); // new
	return r >= 0 ? 0 : r;
}

int BookManager::SetChapter(const char* site, const char* name, const char* author, const char* uri, const char* chapter, const char* datetime)
{
	assert(0 == strchr(chapter, '\''));
	assert(0 == strchr(datetime, '\''));

	int bookid = 0;
	if(0 != GetBookId(name, author, bookid))
		return ERROR_NOTFOUND;

	char sql[2*1024] = {0};
	snprintf(sql, sizeof(sql)-1, 
		"insert into %s (bid, uri, chapter, datetime) values (%d, '%s', '%s', '%s')", 
		site, bookid, uri, chapter, datetime);

	// add new
	AutoThreadLocker locker(g_locker);
	int r = db_insert(m_db, sql);
	if(r < 0)
	{
		snprintf(sql, sizeof(sql)-1, 
			"update %s set uri='%s', chapter='%s', datetime='%s' where bid=%d", 
			site, uri, chapter, datetime, bookid);
		r = db_update(m_db, sql);
	}
	return r >= 0 ? 0 : r;
}

int BookManager::GetLastCheckDatetime(const char* site, char datetime[20])
{
	char sql[1024] = {0};
	snprintf(sql, sizeof(sql)-1, "select max(datetime) from %s", site);

	AutoThreadLocker locker(g_locker);
	return db_query_string(m_db, sql, datetime, 20);
}
