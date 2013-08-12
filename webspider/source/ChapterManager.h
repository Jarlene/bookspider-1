#ifndef _ChapterManager_h_
#define _ChapterManager_h_

#include "sys/sync.hpp"
#include <map>
#include <list>
#include <string>

class ChapterManager
{
	typedef std::map<int, std::list<std::string> > book_t;

	ChapterManager();
	~ChapterManager();

public:
	static ChapterManager& GetInstance();

public:
	int Update(int bookid, const std::list<std::string>& book);

private:
	book_t::iterator GetBook(int bookid);

private:
	book_t m_books;
	ThreadLocker m_locker;
};

#endif /* !_ChapterManager_h_ */
