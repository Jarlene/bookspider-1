#ifndef _IJokeSpider_h_
#define _IJokeSpider_h_

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

typedef int (*OnJoke)(void* param, const char* id, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove);

struct IJokeSpider
{
	virtual ~IJokeSpider(){}

	virtual int GetId() const = 0;
	virtual const char* GetName() const = 0;
	virtual int List() = 0;
};

#endif /* !_IJokeSpider_h_ */
