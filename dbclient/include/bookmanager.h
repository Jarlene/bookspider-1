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

class DBCLIENT_API BookManager
{
public:
	struct Book
	{
		char name[100];		// book name
		char author[100];	// author name
		char uri[128];		// http://www.book.com/books/book.html
		char category[20];	// magic
		char chapter[100];	// book chapter
		char datetime[24];	// 2012-08-21 10:11:32
		int siteid;
		int vote;
	};

	struct BookSite
	{
		int bookid;
		char uri[128];
	};

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

	int QueryBook(int mid, int from, int count, std::vector<std::pair<int, Book> >& books) const;

	int GetBookChapter(int bookid, char chapter1[100], char chapter2[100], char chapter3[100]);
	int SetBookChapter(int bookid, const char* chapter1, const char* chapter2, const char* chapter3);

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

private:
	int db_mid();

private:
	void* m_db;
};

#endif /* !_bookmanager_h_ */
