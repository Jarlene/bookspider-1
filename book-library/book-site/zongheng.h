#ifndef _zongheng_h_
#define _zongheng_h_

#include "book-site.h"

class CZongHeng : public IBookSite
{
public:
	virtual int GetId() const { return 2; }
	virtual const char* GetName() const { return "zongheng"; }
	virtual const char* GetUri(int top) const;
	virtual int GetCount() const { return 5000; }

	virtual int ReadBook(const char* uri, book_info& book);
	virtual int ReadChapter(const char* uri, std::string& chapter);
};

#endif /* !_zongheng_h_ */
