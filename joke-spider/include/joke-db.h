#ifndef _joke_db_h_
#define _joke_db_h_

#include <vector>
#include <string>

typedef struct
{
	unsigned int id;
	std::string author;
	std::string datetime;
	std::string content;
	std::string image;
	int approve;
	int disapprove;
} Joke;

typedef std::vector<Joke> Jokes;

int jokedb_init();
int jokedb_clean();

int jokedb_gettime(const char* website, char datetime[20]);
int jokedb_settime(const char* website, const char* datetime);

int jokedb_insert(const char* website, const Jokes& jokes);

#endif /* !_joke_db_h_ */
