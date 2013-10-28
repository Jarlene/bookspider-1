#ifndef _IJokeSpider_h_
#define _IJokeSpider_h_

#include "joke-db.h"

#define JOKE_SITE_ID	1000000000

struct IJokeSpider
{
	virtual ~IJokeSpider(){}

	virtual int GetId() const = 0;
	virtual const char* GetName() const = 0;
	virtual int List() = 0;
	virtual int GetComment(Comments& comments, unsigned int id) = 0;
};

typedef int (*OnJoke)(void* param, const char* id, const char* icon, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove, int comment);
typedef int (*OnComment)(void* param, const char* icon, const char* user, const char* content);

int joke_get(const IJokeSpider* spider, const char* uri, const char* req, OnJoke callback, void* param);
int joke_comment(const IJokeSpider* spider, const char* uri, const char* req, OnComment callback, void* param);

#endif /* !_IJokeSpider_h_ */
