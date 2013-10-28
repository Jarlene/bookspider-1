#include "cstringext.h"
#include "sys/sock.h"
#include "time64.h"
#include "error.h"
#include "utf8codec.h"
#include "joke-db.h"
#include <stdio.h>

#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"

int main(int argc, char* argv[])
{
	socket_init();
	if(0 != jokedb_init())
		return 0;

	for(int i=1; i<argc; i++)
	{
		if(streq(argv[i], "--list") && i+1<argc)
		{
			IJokeSpider* spider = NULL;
			if(strieq("qiushibaike", argv[i+1]))
			{
				spider = new CQiuShibaiKe();
			}
			else if(strieq("baisibudejie", argv[i+1]))
			{
				int nav = 1;
				if(argc > i+2)
				{
					++i;
					nav = atoi(argv[i+2])
				}
				spider = new CBaiSiBuDeJie(nav);
			}

			if(spider)
				spider->List();

			++i;
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
