#ifndef _17k_h_
#define _17k_h_

#include "book-site.h"

class C17K : public IBookSite
{
public:
	virtual int GetId() const { return 3; }
	virtual const char* GetName() const { return "17k"; }
	virtual const char* GetUri(int top) const;
};

#endif /* !_17k_h_ */
