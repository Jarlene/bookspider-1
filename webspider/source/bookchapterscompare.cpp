#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <list>
#include "mmptr.h"
#include "unicode.h"

typedef std::list<std::string> chapters_t;

bool chapter_find(const char* chapter1, const char* chapter2);

static size_t book_chapter_find(const chapters_t& book, const std::string& chapter)
{
	chapters_t::const_reverse_iterator it = book.rbegin();
	for(size_t i=book.size(); it!= book.rend(); ++it, --i)
	{
		const std::string& ch = *it;
		if(chapter_find(chapter.c_str(), ch.c_str()))
			return i;
	}
	return 0;
}

static int FindBookChaptersHelper(chapters_t::const_reverse_iterator begin,
								  chapters_t::const_reverse_iterator end,
								  chapters_t::const_reverse_iterator begin2,
								  chapters_t::const_reverse_iterator end2)
{
	// check maximum 5-chapters
	int n;
	for(n=0; begin!=end && begin2!=end2 && n<5; ++begin, ++begin2, ++n)
	{
		const std::string& chapter1 = *begin;
		const std::string& chapter2 = *begin2;
		if(!chapter_find(chapter1.c_str(), chapter2.c_str()))
			break;
	}
	return n;
}

///location chapter
///@param[in] book
///@param[in] chapters
///@return book has latest chapter(book newer than chapters)
bool HaveLatestChapter(const std::list<std::string>& book, 
					   const std::list<std::string>& chapters)
{
	assert(chapters.size() > 0);

	//std::list<std::wstring> book2;
	//for(std::list<std::string>::const_iterator it=book.begin(); it!=book.end(); ++it)
	//{
	//	const std::string& chapter = *it;;
	//	mmptr ptr(sizeof(wchar_t)*(chapter.length()+1));
	//	unicode_from_utf8(chapter.c_str(), 0, (wchar_t*)ptr.get(), ptr.capacity());
	//	book2.push_back((const wchar_t*)ptr.get());
	//}

	// locate chapter in books
	size_t i = chapters.size();
	chapters_t::const_reverse_iterator it;
	for(it=chapters.rbegin(); it!=chapters.rend(); ++it)
	{
		assert(!it->empty());
		size_t j = book_chapter_find(book, *it);
		if(j != 0)
		{
			// find it
			// compare chapter number
			return (chapters.size()-i) < (book.size()-j);
		}
	}

	//if(it == chapters.rend())
	//	return false;

	//// check next chapter
	//int n = FindBookChaptersHelper(++bit, book.rend(), ++cit, chapters.rend());
	//if(n+1 == chapters.size())
	//	return true;
	
	return false; // don't find chapter
}
