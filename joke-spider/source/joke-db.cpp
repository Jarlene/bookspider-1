#include "joke-db.h"
#include "dbclient.h"
#include "cstringext.h"

static void* db;
static char buffer[2*1024*1024];

int jokedb_init()
{
	db_init();
	db = db_connect("115.28.51.131", 3306, "joke", "root", "");
	return db ? 0 : -1;
}

int jokedb_clean()
{
	db_disconnect(db);
	db_fini();
	return 0;
}

int jokedb_gettime(const char* website, char datetime[20])
{
	snprintf(buffer, sizeof(buffer)-1, "select datetime from website where website='%s'", website);
	return db_query_string(db, buffer, datetime, 20);
}

int jokedb_settime(const char* website, const char* datetime)
{
	snprintf(buffer, sizeof(buffer)-1, "update website set datetime='%s' where website='%s'", datetime, website);
	return db_update(db, buffer);
}

int jokedb_insert(const char* /*website*/, const Jokes& jokes)
{
	int i = 0;
	std::string sql;
	Jokes::const_iterator it;
	for(it = jokes.begin(); it != jokes.end(); ++it,++i)
	{
		const Joke& joke = *it;
		snprintf(buffer, sizeof(buffer)-1, "(%u, '%s', '%s', '%s', '%s', %d, %d)",
			joke.id, joke.author.c_str(), joke.datetime.c_str(), joke.content.c_str(), joke.image.c_str(), joke.approve, joke.disapprove);

		if(!sql.empty())
			sql += ',';
		sql += buffer;
	}

	sql.insert(0, "insert into joke (id, author, datetime, content, image, approve, disapprove) values ");
	return db_insert(db, sql.c_str());
}
