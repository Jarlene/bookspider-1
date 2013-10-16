#ifndef _IJokeSpider_h_
#define _IJokeSpider_h_

struct IJokeSpider
{
	virtual ~IJokeSpider(){}

	virtual int GetId() const = 0;
	virtual const char* GetName() const = 0;
	virtual int List() = 0;
};

typedef int (*OnJoke)(void* param, const char* id, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove);

int ListJoke(const IJokeSpider* spider, const char* uri, const char* req, OnJoke callback, void* param);

#endif /* !_IJokeSpider_h_ */
