#ifndef _joke_db_h_
#define _joke_db_h_

#include <vector>
#include <string>

typedef struct _Joke
{
	unsigned int id;
	std::string icon;
	std::string author;
	std::string datetime;
	std::string content;
	std::string image;
	int approve;
	int disapprove;
	int comment;
} Joke;

typedef struct _Comment
{
	std::string icon;
	std::string user;
	std::string content;
	int floor;
} Comment;

typedef struct _Comic
{
	unsigned int id;
	std::string title;
	std::string text;
	std::string datetime;
	std::vector<std::string> images;
} Comic;

typedef std::vector<Joke> Jokes;
typedef std::vector<Comic> Comics;
typedef std::vector<Comment> Comments;

int jokedb_init();
int jokedb_clean();

int jokedb_gettime(const char* website, char datetime[20]);
int jokedb_settime(const char* website, const char* datetime);

int jokedb_insert_jokes(const char* website, const Jokes& jokes);
int jokedb_insert_comics(const char* website, const Comics& comics);
int jokedb_insert_comments(const char* website, unsigned int id, const Comments& comments);

int jokedb_query_comment(unsigned int id, char datetime[20], std::string& comment);

#endif /* !_joke_db_h_ */
