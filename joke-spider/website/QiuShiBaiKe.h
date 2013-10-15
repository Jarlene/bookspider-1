#ifndef _QiuShiBaiKe_h_
#define _QiuShiBaiKe_h_

#include "JokeSpider.h"

class CQiuShiBaiKe : public IJokeSpider
{
public:
	virtual int GetId() const { return 101; }
	virtual const char* GetName() const { return "qiushibaike"; }
	virtual int List();
};

#endif /* !_QiuShiBaiKe_h_ */
