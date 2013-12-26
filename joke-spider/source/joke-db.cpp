#include "joke-db.h"
#include "sys/sync.hpp"
#include "dbclient.h"
#include "cstringext.h"
#include "cppstringext.h"
#include <algorithm>

static void* db;
static char buffer[2*1024*1024];
ThreadLocker g_locker;

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
	char sql[256] = {0};
	snprintf(sql, sizeof(sql)-1, "select datetime from website where website='%s'", website);

	AutoThreadLocker locker(g_locker);
	return db_query_string(db, sql, datetime, 20);
}

int jokedb_settime(const char* website, const char* datetime)
{
	char sql[256] = {0};
	snprintf(sql, sizeof(sql)-1, "update website set datetime='%s' where website='%s'", datetime, website);

	AutoThreadLocker locker(g_locker);
	return db_update(db, sql);
}

static int jokedb_insert_text_jokes(const char* /*website*/, const Jokes& jokes)
{
	AutoThreadLocker locker(g_locker);

	int i = 0;
	std::string sql;
	Jokes::const_iterator it;
	for(it = jokes.begin(); it != jokes.end(); ++it,++i)
	{
		const Joke& joke = *it;
		if(!joke.image.empty())
			continue; // ignore images

		snprintf(buffer, sizeof(buffer)-1, 
			"(%u, '%s', '%s', '%s', '%s', %d, %d, '%d')",
			joke.id, joke.author.c_str(), joke.icon.c_str(), joke.datetime.c_str(), joke.content.c_str(), joke.approve, joke.disapprove, joke.comment);

		if(!sql.empty())
			sql += ',';
		sql += buffer;
	}

	if(sql.empty())
		return 0;

	sql.insert(0, "insert into joke_text (id, author, author_icon, datetime, content, approve, disapprove, comment) values ");
	sql += " on duplicate key update approve=values(approve), disapprove=values(disapprove), comment=values(comment)";
	return db_insert(db, sql.c_str());
}

static int jokedb_insert_image_jokes(const char* /*website*/, const Jokes& jokes)
{
	AutoThreadLocker locker(g_locker);

	int i = 0;
	std::string sql;
	Jokes::const_iterator it;
	for(it = jokes.begin(); it != jokes.end(); ++it,++i)
	{
		const Joke& joke = *it;
		if(joke.image.empty())
			continue; // ignore text only joke

		snprintf(buffer, sizeof(buffer)-1, 
			"(%u, '%s', '%s', '%s', '%s', '%s', %d, %d, '%d')",
			joke.id, joke.author.c_str(), joke.icon.c_str(), joke.datetime.c_str(), joke.content.c_str(), joke.image.c_str(), joke.approve, joke.disapprove, joke.comment);

		if(!sql.empty())
			sql += ',';
		sql += buffer;
	}

	if(sql.empty())
		return 0;

	sql.insert(0, "insert into joke_image (id, author, author_icon, datetime, content, image, approve, disapprove, comment) values ");
	sql += " on duplicate key update approve=values(approve), disapprove=values(disapprove), comment=values(comment)";
	return db_insert(db, sql.c_str());
}

int jokedb_insert_jokes(const char* website, const Jokes& jokes)
{
	int i = jokedb_insert_text_jokes(website, jokes);
	i = jokedb_insert_image_jokes(website, jokes);
	return i;
}

static bool joke_approve_compare(const Joke& l, const Joke& r)
{
	return l.approve < r.approve;
}

int jokedb_query_jokes(const char* /*website*/, int images, int hot, Jokes& jokes)
{
	char sql[128] = {0};
	if(1 == images)
		snprintf(sql, sizeof(sql)-1, "select * from joke_image order by datetime desc limit 0,800");
	else
		snprintf(sql, sizeof(sql)-1, "select * from joke_text order by datetime desc limit 0,500");

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(db, sql, result);
	if(r < 0)
		return r;

	while(0 == result.FetchRow())
	{
		Joke joke;
		int id = 0;
		r = result.GetValue("id", id);
		r = result.GetValue("author", joke.author);
		r = result.GetValue("author_icon", joke.icon);
		r = result.GetValue("datetime", joke.datetime);
		r = result.GetValue("content", joke.content);
		r = result.GetValue("image", joke.image);
		r = result.GetValue("approve", joke.approve);
		r = result.GetValue("disapprove", joke.disapprove);
		r = result.GetValue("comment", joke.comment);
		joke.id = (unsigned int)id;
		jokes.push_back(joke);
	}

	if(hot)
	{
		std::sort(jokes.begin(), jokes.end(), joke_approve_compare);
	}

	return 0;
}

int jokedb_insert_comics(const char* /*website*/, const Comics& comics)
{
	AutoThreadLocker locker(g_locker);

	int i = 0;
	std::string sql;
	Comics::const_iterator it;
	for(it = comics.begin(); it != comics.end(); ++it,++i)
	{
		const Comic& comic = *it;
		assert(!comic.images.empty());

		std::string image;
		std::vector<std::string>::const_iterator j;
		for(j = comic.images.begin(); j != comic.images.end(); ++j)
		{
			if(!image.empty())
				image += ",";
			image += *j;
		}
		
		if(image.empty())
			continue;

		snprintf(buffer, sizeof(buffer)-1, 
			"(%u, '%s', '%s', '%s', '%s')",
			comic.id, comic.title.c_str(), image.c_str(), comic.text.c_str(), comic.datetime.c_str());

		if(!sql.empty())
			sql += ',';
		sql += buffer;
	}

	if(sql.empty())
		return 0;

	sql.insert(0, "insert into joke_18plus (id, title, image, text, datetime) values ");
	sql += " on duplicate key update text=values(text)";
	return db_insert(db, sql.c_str());
}

int jokedb_query_comics(const char* /*website*/, Comics& comics)
{
	char sql[128] = {0};
	snprintf(sql, sizeof(sql)-1, "select id, title, text, image, datetime from joke_18plus order by datetime desc limit 0,1000");

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(db, sql, result);
	if(r < 0)
		return r;

	while(0 == result.FetchRow())
	{
		int id = 0;
		Comic comic;
		std::string image;
		r = result.GetValue("id", id);
		r = result.GetValue("title", comic.title);
		r = result.GetValue("text", comic.text);
		r = result.GetValue("image", image);
		r = result.GetValue("datetime", comic.datetime);
		comic.id = id;
		Split(image.c_str(), ',', comic.images);
		comics.push_back(comic);
	}
	return 0;
}

int jokedb_insert_comments(const char* /*website*/, unsigned int id, const Comments& comments)
{
	AutoThreadLocker locker(g_locker);

	int i = 0;
	std::string sql;
	Comments::const_iterator it;
	for(it = comments.begin(); it != comments.end(); ++it,++i)
	{
		const Comment& comment = *it;
		snprintf(buffer, sizeof(buffer)-1, 
			"(%u, '%s', '%s', '%s')",
			id, comment.icon.c_str(), comment.user.c_str(), comment.content.c_str());

		if(!sql.empty())
			sql += ',';
		sql += buffer;
	}

	// clear comment data
	snprintf(buffer, sizeof(buffer)-1, "delete from comment where id=%u", id);
	db_delete(db, buffer);

	sql.insert(0, "insert into comment (id, icon, user, content) values ");
	return db_insert(db, sql.c_str());
}

int jokedb_query_comment(unsigned int id, char datetime[20], std::string& comment)
{
	char sql[128] = {0};
	snprintf(sql, sizeof(sql)-1, "select datetime, comment from comment where id=%u", id);

	DBQueryResult result;
	AutoThreadLocker locker(g_locker);
	int r = db_query(db, sql, result);
	if(r < 0)
		return r;

	r = result.FetchRow();
	if(r < 0)
		return r;

	r = result.GetValue("datetime", datetime, 20);
	r = result.GetValue("comment", comment);
	return r;
}
