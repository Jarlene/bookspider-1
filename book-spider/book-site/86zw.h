#ifndef _86zw_h_
#define _86zw_h_

#include "book-spider.h"

class C86ZW : public IBookSpider
{
public:
	virtual int GetId() const { return 102; }
	virtual const char* GetName() const { return "86zw"; }
	virtual int List(OnBook callback, void* param);
	virtual int Check(OnBook callback, void* param);
	virtual int Search(const char* book, const char* author, char *bookUri);
};

#endif /* !_86zw_h_ */
