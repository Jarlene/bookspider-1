#ifndef _BaiSiBuDeJie_h_
#define _BaiSiBuDeJie_h_

#include "JokeSpider.h"

class CBaiSiBuDeJie : public IJokeSpider
{
public:
	// nav: 1-picture, 2-text only
	CBaiSiBuDeJie(int nav){ m_nav = nav; }

public:
	virtual int GetId() const { return 2; }
	virtual const char* GetName() const { return "baisibudejie"; }
	virtual int List();
	virtual int GetComment(unsigned int id);

private:
	int m_nav;
};

#endif /* !_BaiSiBuDeJie_h_ */
