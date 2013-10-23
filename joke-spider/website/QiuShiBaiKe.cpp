#include "QiuShiBaiKe.h"
#include "cstringext.h"
#include "joke-db.h"
#include <time.h>
#include <math.h>

static int OnList(void* param, const char* id, const char* icon, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove, int comment)
{
	Jokes* jokes = (Jokes*)param;

	const char* p = strrchr(id, '_');
	if(p)
		p = p+1;
	else
		p = id;

	Joke joke;
	joke.id = (unsigned int)atoi(p) + 1 * JOKE_SITE_ID;
	joke.icon = icon;
	joke.author = author;
	joke.datetime = datetime;
	joke.content = content;
	joke.image = image;
	joke.approve = approve;
	joke.disapprove = abs(disapprove);
	joke.comment = comment;
	jokes->push_back(joke);
	return 0;
}

int CQiuShiBaiKe::Late()
{
	time_t v = time(NULL);
	v = v / (5*60);

	char uri[256] = {0};
	for(int page=1; page <= 35; page++)
	{
		// latest update
		snprintf(uri, sizeof(uri)-1, "http://www.qiushibaike.com/history/2013/10/22/page/3", page, (unsigned int)v);

		Jokes jokes;
		int r = joke_get(this, uri, NULL, OnList, &jokes);
		printf("CQiuShiBaiKe::List[%d] joke_get=%d.\n", page, r);
		if(0 != r)
			return r;

		if(jokes.size() < 1)
			continue;

		// save
		r = jokedb_insert_jokes(GetName(), jokes);
		if(r < 0)
			printf("CQiuShiBaiKe::List[%d] jokedb_insert=%d.\n", page, r);

		//Jokes::const_iterator it = jokes.begin();
		//for(size_t i=0; i<jokes.size(); i++, ++it)
		//{
		//	const Joke& joke = *it;
		//	GetComment(joke.id);
		//}
	}

	return 0;
}

int CQiuShiBaiKe::List()
{
	time_t v = time(NULL);
	v = v / (5*60);

	char uri[256] = {0};
	for(int page=1; page <= 35; page++)
	{
		// latest update
		snprintf(uri, sizeof(uri)-1, "http://www.qiushibaike.com/8hr/page/%d?s=%u", page, (unsigned int)v);

		Jokes jokes;
		int r = joke_get(this, uri, NULL, OnList, &jokes);
		printf("CQiuShiBaiKe::List[%d] joke_get=%d.\n", page, r);
		if(0 != r)
			return r;

		if(jokes.size() < 1)
			continue;

		// save
		r = jokedb_insert_jokes(GetName(), jokes);
		if(r < 0)
			printf("CQiuShiBaiKe::List[%d] jokedb_insert=%d.\n", page, r);

		//Jokes::const_iterator it = jokes.begin();
		//for(size_t i=0; i<jokes.size(); i++, ++it)
		//{
		//	const Joke& joke = *it;
		//	GetComment(joke.id);
		//}
	}

	return 0;
}

static int OnGetComment(void* param, const char* icon, const char* user, const char* content)
{
	Comments* comments = (Comments*)param;

	Comment comment;
	comment.icon = icon;
	comment.user = user;
	comment.content = content;
	comments->push_back(comment);
	return 0;
}

int CQiuShiBaiKe::GetComment(unsigned int id)
{
	char uri[256] = {0};
	snprintf(uri, sizeof(uri)-1, "http://www.qiushibaike.com/article/%u", id%(GetId()*JOKE_SITE_ID));

	Comments comments;
	int r = joke_comment(this, uri, NULL, OnGetComment, &comments);
	if(r < 0)
	{
		printf("CQiuShiBaiKe::GetComment[%u] error=%d.\n", id, r);
		return r;
	}

	r = jokedb_insert_comments(GetName(), id, comments);
	if(r < 0)
		printf("CQiuShiBaiKe::GetComment[%u] save comment error=%d\n", id, r);

	return r;
}
