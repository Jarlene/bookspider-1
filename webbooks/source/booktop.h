#ifndef _booktop_h_
#define _booktop_h_

#include <map>
#include <vector>
#include <string>

class BookTop
{
public:
	int AddBook(const char* sitename, int bookid, int vote);
	
	int Save(const char* filename);

private:
	int Sort();

private:
	struct BookInfo
	{
		int bookid;
		int vote;
	};

	friend bool BookInfoGreater(const BookInfo& l, const BookInfo& r);
	typedef std::vector<BookInfo> TBooks;
	typedef std::map<std::string, TBooks> TBookSites;
	TBookSites m_booksites;
	TBooks m_booktop;
};

#endif /* !_booktop_h_ */
