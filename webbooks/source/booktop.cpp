#include "booktop.h"
#include "xmlutil.h"
#include <stdio.h>
#include <errno.h>
#include <algorithm>
#include "StrConvert.h"
#include "bookmanager.h"

int BookTop::AddBook(const char* sitename, int bookid, int vote)
{
	TBookSites::iterator it = m_booksites.find(sitename);
	if(it == m_booksites.end())
	{
		it = m_booksites.insert(std::make_pair(sitename, TBooks())).first;
	}
	
	BookInfo book;
	book.vote = vote;
	book.bookid = bookid;
	
	TBooks& books = it->second;
	books.push_back(book);
	return 0;
}

inline bool BookInfoGreater(const BookTop::BookInfo& l, const BookTop::BookInfo& r)
{
	return l.vote > r.vote;
}

static double GetBookSiteFactor(const char* sitename)
{
	static std::map<std::string, double> s_factors;
	if(0 == s_factors.size())
	{
		s_factors.insert(std::make_pair("qidian", 0.5)); // total vote / total vip
		s_factors.insert(std::make_pair("zongheng", 0.8));
		s_factors.insert(std::make_pair("", 0.2));
	}

	std::map<std::string, double>::iterator it;
	it = s_factors.find(sitename);
	if(it != s_factors.end())
		return it->second;
	return s_factors.find("")->second;
}

int BookTop::Sort()
{
	m_booktop.clear();
	
	for(TBookSites::const_iterator it=m_booksites.begin(); it!=m_booksites.end(); ++it)
	{
		const std::string& sitename = it->first;
		const TBooks& books = it->second;
		double factor = GetBookSiteFactor(sitename.c_str());

		for(TBooks::const_iterator j=books.begin(); j!=books.end(); ++j)
		{
			const BookInfo& info = *j;
			BookInfo item;
			item.bookid = info.bookid;
			item.vote = int(info.vote * factor);
			m_booktop.push_back(item);
		}
	}

	std::sort(m_booktop.begin(), m_booktop.end(), BookInfoGreater);
	return m_booktop.size();
}

int BookTop::Save(const char* filename)
{
	Sort();

	FILE* fp = fopen(filename, "w");
	if(!fp)
		return -(int)errno;

	const char* xmldeclaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	fwrite(xmldeclaration, 1, strlen(xmldeclaration), fp);
	fwrite("<top>", 1, 5, fp);

	int i=0;
	BookManager* bookmgr = BookManager::FetchBookManager();
	for(TBooks::const_iterator it=m_booktop.begin(); it!=m_booktop.end()&&i<500; ++it, ++i)
	{
		const BookInfo& info = *it;

		BookManager::Book book;
		if(bookmgr->GetBookInfo(info.bookid, book))
			continue; // not found

		std::string xml;
		XmlTag2(xml, "name", book.name);
		XmlTag2(xml, "author", book.author);
		XmlTag2(xml, "datetime", book.datetime);
		XmlTag2(xml, "chapter", book.chapter);
		XmlTag2(xml, "category", book.category);
		XmlTag2(xml, "vote", ToString(info.vote));

		fwrite("<book>", 1, 6, fp);
		fwrite(xml.c_str(), 1, xml.length(), fp);
		fwrite("</book>", 1, 7, fp);
	}

	fwrite("</top>", 1, 6, fp);
	fclose(fp);
	return 0;
}
