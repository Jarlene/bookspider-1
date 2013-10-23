#ifndef _QiuShiBaiKe_h_
#define _QiuShiBaiKe_h_

#include "JokeSpider.h"

class CQiuShiBaiKe : public IJokeSpider
{
public:
	virtual int GetId() const { return 1; }
	virtual const char* GetName() const { return "qiushibaike"; }
	virtual int List();
	virtual int GetComment(unsigned int id);

private:
	int Late();
};

#endif /* !_QiuShiBaiKe_h_ */
