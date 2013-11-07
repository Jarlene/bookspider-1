#include "BaiSiBuDeJie.h"
#include "cstringext.h"
#include "sys/system.h"
#include "joke-db.h"
#include <time.h>

static int OnList(void* param, const char* id, const char* icon, const char* author, const char* datetime, const char* content, const char* image, int approve, int disapprove, int comment)
{
	Jokes* jokes = (Jokes*)param;

	const char* p = strrchr(id, '-');
	if(p)
		p = p+1;
	else
		p = id;

	Joke joke;
	joke.id = (unsigned int)atoi(p) + 2 * JOKE_SITE_ID;
	joke.icon = icon;
	joke.author = author;
	joke.datetime.assign(datetime, 19);
	joke.content = content;
	joke.image = image;
	joke.approve = approve;
	joke.disapprove = disapprove;
	joke.comment = comment;
	jokes->push_back(joke);
	return 0;
}

int CBaiSiBuDeJie::Check()
{
	int r = joke_check(this, "http://www.budejie.com/detail.php?id=3630394&nav=1#main_conter", NULL);
	printf("CBaiSiBuDeJie::Check=%d.\n", r);
	return r;
}

int CBaiSiBuDeJie::List()
{
	char uri[256] = {0};
	for(int page=1; page <= 100; page++)
	{
		// latest update
		if(2 == m_nav)
			snprintf(uri, sizeof(uri)-1, "http://www.budejie.com/xcs.php?page=%d", page);
		else
			snprintf(uri, sizeof(uri)-1, "http://www.budejie.com/index.php?page=%d", page);

		Jokes jokes;
		int r = joke_list(this, uri, NULL, OnList, &jokes);
		printf("CBaiSiBuDeJie::List[%d] joke_get=%d.\n", page, r);
		if(0 != r)
			return r;

		if(jokes.size() < 1)
			continue;

		// save
		r = jokedb_insert_jokes(GetName(), jokes);
		if(r < 0)
			printf("CBaiSiBuDeJie::List[%d] jokedb_insert=%d.\n", page, r);

		system_sleep(5000);
	}

	return 0;
}

int CBaiSiBuDeJie::Hot()
{
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

int CBaiSiBuDeJie::GetComment(Comments& comments, unsigned int id)
{
	char uri[256] = {0};
	for(int i=1; 1; i++)
	{
		int n = comments.size();
		snprintf(uri, sizeof(uri)-1, "http://www.budejie.com/detail.php?id=%u&nav=%d&page=%d", id%(GetId()*JOKE_SITE_ID), m_nav, i);
		int r = joke_comment(this, uri, NULL, OnGetComment, &comments);
		if(r < 0 || comments.size()==n)
			break;
	}

	return 0;
}
