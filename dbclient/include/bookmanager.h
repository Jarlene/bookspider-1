#ifndef _bookmanager_h_
#define _bookmanager_h_

#include <vector>
#include <string>
#include "dllexport.h"

#ifdef DBCLIENT_EXPORTS
	#define DBCLIENT_API DLL_EXPORT_API
#else
	#define DBCLIENT_API DLL_IMPORT_API
#endif

#define BOOK_ID	100000000

// top type
enum EBookTop{ 
	ETT_ALL_VIEW=1, // all page view
	ETT_ALL_MARK,	// all bookmark
	ETT_ALL_VOTE,	// all user vote
	ETT_MONTH_VIEW,	// month
	ETT_MONTH_MARK,
	ETT_MONTH_VOTE,
	ETT_WEEK_VIEW,	// week
	ETT_WEEK_MARK,
	ETT_WEEK_VOTE,
};

class DBCLIENT_API BookManager
{
public:
	struct Book
	{
		int id;				// book id (siteid * BOOK_ID + book)
		char name[120];		// book name
		char author[120];	// author name
		char uri[128];		// http://www.book.com/books/book.html
		char category[20];	// magic
		char chapter[180];	// book chapter
		char datetime[24];	// 2012-08-21 10:11:32
		int vote;
	};

	struct BookSite
	{
		int bookid;
		char uri[128];
	};

	typedef std::vector<Book> Books;

private:
	BookManager();
	~BookManager();

public:
	static BookManager* FetchBookManager();

public:
	///add or update book
	///@param[in] book book information
	///@return 0-success, 1-update, <0-error
	int AddBook(const Book& book);

	/// find book
	///@param[out] book book information
	///@param[out] bookid book id
	///@return 0-success,<0-error
	int GetBookId(const char* name, const char* author, int& bookid) const;
	int GetBookInfo(const char* name, const char* author, Book& book) const;
	int GetBookInfo(int bookid, Book& book) const;

	int QueryBook(int mid, int from, int count, Books& books) const;

	///add or update book site
	///@param[in] bookid book id
	///@param[in] site book site information
	///@return 0-success,<0-error
	int AddBookSite(int bookid, const char* site, const char* bookuri, const char* indexuri);

	/// delete book site
	///@param[in] bookid book id
	///@param[in] uri book site uri
	///@return 0-success,<0-error
	//int ClearBookSite(int bookid);

	bool CheckBookSite(int bookid, const char* site) const;

	int QueryBookFromSite(const char* site, int from, int count, std::vector<BookSite>& books);

	int SetBookSiteChapter(const char* site, int bookid, const char* chapter);

	// books.siteid must all the same value
	int SetTopBooks(EBookTop top, const Books& books);

	// name: book name
	// author: book author
	// chapter: book last chapter
	int SetChapter(const char* site, const char* name, const char* author, const char* uri, const char* chapter, const char* datetime);

	int GetLastCheckDatetime(const char* site, char datetime[20]);

private:
	int db_mid();
	bool HaveBook(int bookid);

private:
	void* m_db;
};

#endif /* !_bookmanager_h_ */
