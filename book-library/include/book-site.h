#ifndef _book_site_h_
#define _book_site_h_

#include "bookmanager.h"

struct IBookSite
{
	virtual ~IBookSite(){}

	virtual int GetId() const = 0;
	virtual const char* GetName() const = 0;
	virtual const char* GetUri(int top) const = 0;
};

typedef BookManager::Book book_t;

typedef int (*book_site_spider_fcb)(void* param, const book_t* book);

int ListBook(IBookSite* site, int top, book_site_spider_fcb callback, void* param);

#endif /* !_book_site_h_ */
