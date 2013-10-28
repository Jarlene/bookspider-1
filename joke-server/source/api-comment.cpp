#include "web-session.h"
#include "joke-comment.h"
#include "JokeSpider.h"
#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"

static int GetComment(Comments& comments, unsigned int id, int /*content*/)
{
	IJokeSpider* spider = NULL;
	if(id / JOKE_SITE_ID == 1)
		spider = new CQiuShiBaiKe();
	else if(id / JOKE_SITE_ID == 2)
		spider = new CBaiSiBuDeJie(1);
	else
		return ERROR_PARAM;

	int r = spider->GetComment(comments, id % JOKE_SITE_ID);
	if(r < 0)
		return r;

	// update database
	//r = jokecomment_insert(id, time64_now(), comments);
	if(r < 0)
		printf("CQiuShiBaiKe::GetComment[%u] save comment error=%d\n", id, r);

	return r;
}

int WebSession::OnComment(jsonobject& reply)
{
	unsigned int id;
	time64_t datetime;
	std::string comment;

	if(!m_params.Get("id", (int&)id))
		return ERROR_PARAM;

	int content = m_params.Get2("content", 1);

	int r = jokecomment_query(id, datetime, comment);
	if(0 == r)
	{
		// valid if in 10-minutes
		if(datetime + 10*60 > time64_now())
		{
			reply.add("comment", comment);
			return 0;
		}
	}

	Comments comments;
	r = GetComment(comments, id, content);
	printf("WebSession::OnComment get comment(%u) => %d\n", id, r);

	r = jokecomment_query(id, datetime, comment);
	if(r < 0)
	{
		printf("WebSession::OnComment re-query comment(%u) => %d\n", id, r);
		return r;
	}

	reply.add("comment", comment);
	return 0;
}
