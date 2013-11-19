#include "BaoZou.h"
#include "cstringext.h"
#include "sys/system.h"
#include "error.h"
#include "config.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "web-translate.h"

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
	joke.image = strstr(image, "1px.jpg")?"":image;
	joke.approve = approve;
	joke.disapprove = disapprove;
	joke.comment = comment;
	jokes->push_back(joke);
	return 0;
}

int CBaoZou::List()
{
	char uri[256] = {0};
	for(int page=1; page <= 100; page++)
	{
		snprintf(uri, sizeof(uri)-1, "http://baozoumanhua.com/groups/1/hottest/8hr/page/%d", page);

		Jokes jokes;
		int r = joke_list(this, uri, NULL, OnList, &jokes);
		printf("CBaoZou::List[%d] joke_get=%d.\n", page, r);
		if(0 != r)
			return r;

		if(jokes.size() < 1)
			continue;

		// save
		r = jokedb_insert_jokes(GetName(), jokes);
		if(r < 0)
			printf("CBaoZou::List[%d] jokedb_insert=%d.\n", page, r);

		system_sleep(5000);
	}

	return 0;
}

int CBaoZou::Hot()
{
	return 0;
}

int CBaoZou::Check()
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

int CBaoZou::GetComment(Comments& comments, unsigned int id)
{
	char uri[256] = {0};
	for(int i=1; 1; i++)
	{
		size_t n = comments.size();
		snprintf(uri, sizeof(uri)-1, "http://baozoumanhua.com/articles/%u.html", id%(GetId()*JOKE_SITE_ID), m_nav, i);
		int r = joke_comment(this, uri, NULL, OnGetComment, &comments);
		if(r < 0 || comments.size()==n)
			break;
	}
	return 0;
}
