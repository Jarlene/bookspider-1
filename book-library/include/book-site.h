#ifndef _book_site_h_
#define _book_site_h_

#include "bookmanager.h"

typedef std::vector<std::string> Chapters;
typedef std::vector<Chapters> Sections;

typedef struct _book_info
{
	char begintime[24];
	char endtime[24];
	char category[20];
	Sections sections;
} book_info;

struct IBookSite
{
	virtual ~IBookSite(){}

	virtual int GetId() const = 0;
	virtual const char* GetName() const = 0;
	virtual const char* GetUri(int top) const = 0;
	virtual int GetCount() const = 0;

	virtual int ReadBook(const char* uri, book_info& book) = 0;
	virtual int ReadChapter(const char* uri, std::string& chapter) = 0;
};

typedef BookManager::Book book_t;

typedef int (*book_site_spider_fcb)(void* param, const book_t* book);

int ListBook(IBookSite* site, int top, book_site_spider_fcb callback, void* param);

int read_book(IBookSite* site, const char* uri, const char* req, book_info& book);
int read_index(IBookSite* site, const char* uri, const char* req, book_info& book);
int read_chapter(IBookSite* site, const char* uri, const char* req, std::string& chapter);

#endif /* !_book_site_h_ */
