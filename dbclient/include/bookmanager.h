#ifndef _bookmanager_h_
#define _bookmanager_h_

#include <vector>
#include <string>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
	#define DBCLIENT_API_EXPORT __declspec(dllexport)
	#define DBCLIENT_API_IMPORT __declspec(dllimport)
#else
	#if __GNUC__ >= 4
		#define DBCLIENT_API_EXPORT __attribute__((visibility ("default")))
		#define DBCLIENT_API_IMPORT __attribute__((visibility ("default")))
	#else
		#define DBCLIENT_API_EXPORT
		#define DBCLIENT_API_IMPORT
	#endif
#endif

#ifdef DBCLIENT_EXPORTS
	#define DBCLIENT_API DBCLIENT_API_EXPORT
#else
	#define DBCLIENT_API DBCLIENT_API_IMPORT
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
