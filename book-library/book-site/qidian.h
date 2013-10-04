#ifndef _qidian_h_
#define _qidian_h_

#include "book-site.h"

class CQiDian : public IBookSite
{
public:
	virtual int GetId() const { return 1; }
	virtual const char* GetName() const { return "qidian"; }
	virtual const char* GetUri(int top) const;
};

#endif /* !_qidian_h_ */
