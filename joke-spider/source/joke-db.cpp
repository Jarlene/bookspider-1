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

static int jokedb_insert_text_jokes(const char* /*website*/, const Jokes& jokes)
{
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

int jokedb_insert_comments(const char* /*website*/, unsigned int id, const Comments& comments)
{
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
	snprintf(buffer, sizeof(buffer)-1, "delete from comment where id=%d", id);
	db_delete(db, buffer);

	sql.insert(0, "insert into comment (id, icon, user, content) values ");
	return db_insert(db, sql.c_str());
}
