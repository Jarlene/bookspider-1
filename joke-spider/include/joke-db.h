#ifndef _joke_db_h_
#define _joke_db_h_

#include <vector>
#include <string>

typedef struct
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

typedef struct
{
	std::string icon;
	std::string user;
	std::string content;
} Comment;

typedef std::vector<Joke> Jokes;
typedef std::vector<Comment> Comments;

int jokedb_init();
int jokedb_clean();

int jokedb_gettime(const char* website, char datetime[20]);
int jokedb_settime(const char* website, const char* datetime);

int jokedb_insert_jokes(const char* website, const Jokes& jokes);
int jokedb_insert_comments(const char* website, unsigned int id, const Comments& comments);

int jokedb_query_comment(unsigned int id, char datetime[20], std::string& comment);

#endif /* !_joke_db_h_ */
