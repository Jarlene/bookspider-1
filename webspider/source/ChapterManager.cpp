#include "ChapterManager.h"
#include "bookmanager.h"
#include <assert.h>

bool HaveLatestChapter(const std::list<std::string>& book, const std::list<std::string>& chapters);

ChapterManager::ChapterManager()
{
}

ChapterManager::~ChapterManager()
{
}

ChapterManager& ChapterManager::GetInstance()
{
	static ChapterManager s_mgr;
	return s_mgr;
}

int ChapterManager::Update(int bookid, const std::list<std::string>& book)
{
	assert(book.size() >= 3);
	AutoThreadLocker locker(m_locker);
	book_t::iterator it = GetBook(bookid);
	if(it == m_books.end())
		return -1; // book not found

	std::list<std::string>& chapters = it->second;
	assert(chapters.size() <= 3);
	if(chapters.size()>1 && !HaveLatestChapter(book, chapters))
		return 0;
	
	// the latest, update chapter
	std::list<std::string>::const_reverse_iterator rit = book.rbegin();
	const std::string& chapter3 = *rit++;
	const std::string& chapter2 = *rit++;
	const std::string& chapter1 = *rit++;
	assert(!chapter1.empty() && !chapter2.empty() && !chapter3.empty());

	chapters.clear();
	chapters.push_back(chapter1);
	chapters.push_back(chapter2);
	chapters.push_back(chapter3);

	// update database
	BookManager* bookmgr = BookManager::FetchBookManager();
	int r = bookmgr->SetBookChapter(bookid, chapter1.c_str(), chapter2.c_str(), chapter3.c_str());
	return r;
}

ChapterManager::book_t::iterator ChapterManager::GetBook(int bookid)
{
	AutoThreadLocker locker(m_locker);
	book_t::iterator it = m_books.find(bookid);
	if(it != m_books.end())
		return it;

	BookManager* bookmgr = BookManager::FetchBookManager();

	char chapter1[100], chapter2[100], chapter3[100];
	if(0!=bookmgr->GetBookChapter(bookid, chapter1, chapter2, chapter3))
		return m_books.end();

	std::list<std::string> chapters;
	if(strlen(chapter1)) chapters.push_back(chapter1);
	if(strlen(chapter2)) chapters.push_back(chapter2);
	if(strlen(chapter3)) chapters.push_back(chapter3);
	std::pair<book_t::iterator, bool> pr;
	pr = m_books.insert(std::make_pair(bookid, chapters));
	if(!pr.second)
		return m_books.end(); // insert failed

	return pr.first;
}
