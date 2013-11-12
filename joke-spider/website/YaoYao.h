#ifndef _yaoyao_h_
#define _yaoyao_h_

#include "JokeSpider.h"

class CYaoYao : public IJokeSpider
{
public:
	virtual int GetId() const { return 3; }
	virtual const char* GetName() const { return "yaoyao"; }
	virtual int Check();
	virtual int List();
	virtual int Hot();
	virtual int GetComment(Comments& comments, unsigned int id);

private:
	int Late();
};

#endif /* !_yaoyao_h_ */
