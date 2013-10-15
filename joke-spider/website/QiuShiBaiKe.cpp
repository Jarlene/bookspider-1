#include "QiuShiBaiKe.h"
#include "cstringext.h"
#include "sys/system.h"
#include <time.h>

static int OnHandle(void* param, const char* id, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove)
{
	Jokes* jokes = (Jokes*)param;

	const char* p = strrchr(id, '_');
	if(p)
		p = p+1;
	else
		p = id;

	Joke joke;
	joke.id = (unsigned int)atoi(p);
	joke.author = author;
	joke.datetime = datetime;
	joke.content = content;
	joke.image = image;
	joke.approve = approve;
	joke.disapprove = disapprove;
	jokes->push_back(joke);
	return 0;
}

int InsertJoke(const Jokes& jokes);
int ListJoke(const IJokeSpider* spider, const char* uri, const char* req, OnJoke callback, void* param);

int CQiuShiBaiKe::List()
{
	time_t v = time(NULL);
	v = v / (5*60);

	char uri[256] = {0};
	for(int page=1; page < 35; page++)
	{
		Jokes jokes;
		snprintf(uri, sizeof(uri)-1, "http://www.qiushibaike.com/8hr/page/%d?s=%d", page, v);
		int r = ListJoke(this, uri, NULL, OnHandle, &jokes);
		if(0 != r)
			return r;

		r = InsertJoke(jokes);

		printf("CQiuShiBaiKe page: %d, insert: %d\n", page, r);
		system_sleep(5000);
	}

	return 0;
}
