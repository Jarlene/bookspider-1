#include "cstringext.h"
#include "sys/sock.h"
#include "time64.h"
#include "error.h"
#include "utf8codec.h"
#include "dbclient.h"
#include <stdio.h>

#include "QiuShiBaiKe.h"

static void* db;
static char buffer[2*1024*1024];

int InsertJoke(const Jokes& jokes)
{
	std::string sql;
	Jokes::const_iterator it;
	for(it = jokes.begin(); it != jokes.end(); ++it)
	{
		const Joke& joke = *it;
		snprintf(buffer, sizeof(buffer)-1, "(%u, '%s', '%s', '%s', '%s', %d, %d)",
			joke.id, joke.author.c_str(), joke.datetime.c_str(), joke.content.c_str(), joke.image.c_str(), joke.approve, joke.disapprove);

		if(!sql.empty())
			sql += ',';
		sql += buffer;
	}

	sql.insert(0, "insert into joke (id, author, datetime, content, image, approve, disapprove) values ");
	int r = db_insert(db, sql.c_str());
	return r;
}

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
	db_init();
	db = db_connect("115.28.51.131", 3306, "joke", "root", "");
	if(!db)
	{
		system("pause");
		return 0;
	}

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

	db_disconnect(db);
	db_fini();
	socket_cleanup();
	return 0;
}
