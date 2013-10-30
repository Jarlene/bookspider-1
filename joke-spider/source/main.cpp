#include "cstringext.h"
#include "sys/sock.h"
#include "time64.h"
#include "error.h"
#include "utf8codec.h"
#include "joke-db.h"
#include <stdio.h>

#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"

static IJokeSpider* MakeSpider(const char* name)
{
	IJokeSpider* spider = NULL;
	if(strieq("qiushibaike", name))
	{
		spider = new CQiuShiBaiKe();
	}
	else if(strieq("baisibudejie", name))
	{
		spider = new CBaiSiBuDeJie(1);
	}
	
	return spider;
}

int main(int argc, char* argv[])
{
	IJokeSpider* spider = NULL;

	socket_init();
	if(0 != jokedb_init())
		return 0;

	for(int i=1; i<argc; i++)
	{
		if(streq(argv[i], "--website") && i+1<argc)
		{
			spider = MakeSpider(argv[++i]);
		}
		else if(streq(argv[i], "--list"))
		{
			if(spider)
				spider->List();
			break;
		}
		else if(streq(argv[i], "--comment") && i+1<argc)
		{
			int comment = atoi(argv[++i]);
			if(spider)
			{
				Comments comments;
				spider->GetComment(comments, comment);
			}
			break;
		}
		else
		{
			printf("Joke-Spider --website [qiushibaike|baisibudejie] [--list] [--comment id]\n");
			break;
		}
	}

	jokedb_clean();
	socket_cleanup();
	return 0;
}
