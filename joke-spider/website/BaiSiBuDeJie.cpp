#include "BaiSiBuDeJie.h"
#include "cstringext.h"
#include "joke-db.h"
#include <time.h>

static int OnList(void* param, const char* id, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove)
{
	Jokes* jokes = (Jokes*)param;

	const char* p = strrchr(id, '-');
	if(p)
		p = p+1;
	else
		p = id;

	Joke joke;
	joke.id = (unsigned int)atoi(p) + 2 * JOKE_SITE_ID;
	joke.author = author;
	joke.datetime.assign(datetime, 19);
	joke.content = content;
	joke.image = image;
	joke.approve = approve;
	joke.disapprove = disapprove;
	jokes->push_back(joke);
	return 0;
}

int CBaiSiBuDeJie::List()
{
	char uri[256] = {0};
	for(int page=1; page <= 35; page++)
	{
		// latest update
		if(2 == m_nav)
			snprintf(uri, sizeof(uri)-1, "http://budejie.com/xcs.php?page=%d", page);
		else
			snprintf(uri, sizeof(uri)-1, "http://budejie.com/index.php?page=%d", page);

		Jokes jokes;
		int r = joke_get(this, uri, NULL, OnList, &jokes);
		printf("CBaiSiBuDeJie::List[%d] joke_get=%d.\n", page, r);
		if(0 != r)
			return r;

		if(jokes.size() < 1)
			continue;

		// save
		r = jokedb_insert_jokes(GetName(), jokes);
		if(r < 0)
			printf("CBaiSiBuDeJie::List[%d] jokedb_insert=%d.\n", page, r);

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

int CBaiSiBuDeJie::GetComment(unsigned int id)
{
	char uri[256] = {0};
	snprintf(uri, sizeof(uri)-1, "http://budejie.com/detail.php?id=%u&nav=%d", id%(GetId()*JOKE_SITE_ID), m_nav);

	Comments comments;
	int r = joke_comment(this, uri, NULL, OnGetComment, &comments);
	if(r < 0)
	{
		printf("CBaiSiBuDeJie::GetComment[%u] error=%d.\n", r);
		return r;
	}

	r = jokedb_insert_comments(GetName(), id, comments);
	if(r < 0)
		printf("CBaiSiBuDeJie::GetComment[%u] save comment error=%d\n", r);

	return r;
}
