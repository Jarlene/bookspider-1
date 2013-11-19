#ifndef _baozou_h_
#define _baozou_h_

#include "JokeSpider.h"

class CBaoZou : public IJokeSpider
{
public:
	virtual int GetId() const { return 4; }
	virtual const char* GetName() const { return "baozou"; }
	virtual int Check();
	virtual int List();
	virtual int Hot();
	virtual int GetComment(Comments& comments, unsigned int id);

private:
	int Late();
};

#endif /* !_baozou_h_ */
