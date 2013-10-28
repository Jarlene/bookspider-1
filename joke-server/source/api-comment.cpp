#include "web-session.h"
#include "joke-comment.h"
#include "JokeSpider.h"
#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"

static void JsonEncode(const Comments& comments, std::string& comment)
{
	jsonarray jarr;
	Comments::const_iterator it;
	for(it = comments.begin(); it != comments.end(); ++it)
	{
		const Comment& comment = it;

		jsonobject json;
		json.add("icon", comment.icon);
		json.add("user", comment.user);
		json.add("comment", comment.content);
		jarr.add(json);
	}

	comment = jarr.json();
}

static int GetComment(unsigned int id, std::string& comment)
{
	IJokeSpider* spider = NULL;
	if(id / JOKE_SITE_ID == 1)
		spider = new CQiuShiBaiKe();
	else if(id / JOKE_SITE_ID == 2)
		spider = new CBaiSiBuDeJie(1);
	else
		return ERROR_PARAM;

	Comments comments;
	int r = spider->GetComment(comments, id % JOKE_SITE_ID);
	if(r < 0)
		return r;

	JsonEncode(comments);
	return r;
}

static int QueryComment(unsigned int id, std::string& comment)
{
	time64_t datetime;
	int r = jokecomment_query(id, datetime, comment);
	if(0 == r && datetime + 10*60 > time64_now())
		return 0; // valid if in 10-minutes

	// update from website
	r = GetComment(id, comment);
	if(0 == r)
		r = jokecomment_insert(id, time64_now(), comment); // update database
	return r;
}

int WebSession::OnComment(jsonobject& reply)
{
	unsigned int id;
	if(!m_params.Get("id", (int&)id))
		return ERROR_PARAM;

	//int content = m_params.Get2("content", 1);

	std::string comment;
	int r = QueryComment(id, comment);
	printf("WebSession::OnComment get comment(%u) => %d\n", id, r);

	reply.add("comment", comment);
	return 0;
}
