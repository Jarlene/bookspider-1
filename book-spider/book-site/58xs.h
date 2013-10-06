#ifndef _58xs_h_
#define _58xs_h_

#include "book-spider.h"

class C58XS : public IBookSpider
{
public:
	virtual int GetId() const { return 101; }
	virtual const char* GetName() const { return "58xs"; }
	virtual int List(OnBook callback, void* param);
	virtual int Check(OnBook callback, void* param);
	virtual int Search(const char* book, const char* author, char *bookUri);
};

#endif /* !_58xs_h_ */
