#ifndef _luoqiu_h_
#define _luoqiu_h_

#include "book-spider.h"

class CLuoQiu : public IBookSpider
{
public:
	virtual int GetId() const { return 103; }
	virtual const char* GetName() const { return "luoqiu"; }
	virtual int List(OnBook callback, void* param);
	virtual int Check(OnBook callback, void* param);
	virtual int Search(const char* book, const char* author, char *bookUri);
};

#endif /* !_luoqiu_h_ */
