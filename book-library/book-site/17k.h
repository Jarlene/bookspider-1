#ifndef _17k_h_
#define _17k_h_

#include "book-site.h"

class C17K : public IBookSite
{
public:
	virtual int GetId() const { return 3; }
	virtual const char* GetName() const { return "17k"; }
	virtual const char* GetUri(int top) const;
	virtual int GetCount() const { return 2000; }

	virtual int ReadBook(const char* uri, book_info& book);
	virtual int ReadChapter(const char* uri, std::string& chapter);
};

#endif /* !_17k_h_ */
