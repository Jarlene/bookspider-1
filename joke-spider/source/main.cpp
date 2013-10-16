#include "cstringext.h"
#include "sys/sock.h"
#include "time64.h"
#include "error.h"
#include "utf8codec.h"
#include "joke-db.h"
#include <stdio.h>

#include "QiuShiBaiKe.h"

static IJokeSpider* CreateSpider(const char* site)
{
	if(strieq(site, "qiushibaike")) return new CQiuShiBaiKe();
	//else if(strieq(site, "86zw")) return new C86ZW();
	//else if(strieq(site, "luoqiu")) return new CLuoQiu();

	return NULL;
}

static int JokeList(const char* site)
{
	IJokeSpider* spider = CreateSpider(site);
	if(NULL == spider)
	{
		printf("don't find %s\n", site);
		return ERROR_NOTFOUND;
	}

	return spider->List();
}

int main(int argc, char* argv[])
{
	socket_init();
	if(0 != jokedb_init())
		return 0;

	for(int i=1; i<argc; i++)
	{
		if(streq(argv[i], "--list") && i+1<argc)
		{
			JokeList(argv[++i]);
		}
		else
		{
			printf("Joke-Spider --list site\n");
			break;
		}
	}

	jokedb_clean();
	socket_cleanup();
	return 0;
}
