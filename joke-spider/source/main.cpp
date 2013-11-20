#include "cstringext.h"
#include "sys/sock.h"
#include "time64.h"
#include "error.h"
#include "utf8codec.h"
#include "joke-db.h"
#include "http-proxy.h"
#include <stdio.h>

#ifdef OS_LINUX
#include <signal.h>
#endif

#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"
#include "YaoYao.h"
#include "BaoZou.h"

int config_proxy_load();

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
	else if(strieq("baisibudejie-xcs", name))
	{
		spider = new CBaiSiBuDeJie(2);
	}
	else if(strieq("yaoyao", name))
	{
		spider = new CYaoYao();
	}
	else if(strieq("baozou", name))
	{
		spider = new CBaoZou();
	}
	
	return spider;
}

int main(int argc, char* argv[])
{
#if defined(OS_LINUX)
	/* ignore pipe signal */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, 0);
	sigaction(SIGPIPE, &sa, 0);
#endif

	IJokeSpider* spider = NULL;

	// use proxy
	config_proxy_load();
	http_proxy_add_pattern("*.budejie.com");
	http_proxy_add_pattern("*.qiushibaike.com");
	http_proxy_add_pattern("*.yyxj8.com");

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
		else if(streq(argv[i], "--hot"))
		{
			if(spider)
				spider->Hot();
			break;
		}
		else if(streq(argv[i], "--check"))
		{
			if(spider)
			{
				spider->Check();
			}
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
			printf("Joke-Spider --website [baozou|yaoyao|qiushibaike|baisibudejie|baisibudejie-xcs] [--list | --hot | --check | --comment id]\n");
			break;
		}
	}

	jokedb_clean();
	socket_cleanup();
	return 0;
}
