#include "web-session.h"
#include "sys/sync.hpp"
#include "joke-comment-db.h"
#include "sys-task-queue.h"
#include "JokeSpider.h"
#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"
#include "BaoZou.h"
#include "libct/auto_ptr.h"
#include <map>
#include <list>

extern sys_task_queue_t g_taskQ;

typedef std::list<WebSession*> TSessions;
typedef std::map<unsigned int, TSessions> TComments;
static ThreadLocker s_locker;
static TComments s_comments;

struct TCommentArgs
{
	unsigned int id;
};

static void JsonEncode(const Comments& comments, std::string& comment)
{
	jsonarray jarr;
	Comments::const_iterator it;
	for(it = comments.begin(); it != comments.end(); ++it)
	{
		const Comment& comment = *it;

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
	else if(id / JOKE_SITE_ID == 4)
		spider = new CBaoZou();
	else
		return ERROR_PARAM;

	Comments comments;
	int r = spider->GetComment(comments, id % JOKE_SITE_ID);
	if(r < 0)
		return r;

	JsonEncode(comments, comment);
	return r;
}

/// @param[in] id session id
/// @return true-new request session, false-exist other session
static bool PushSession(unsigned int id, WebSession* session)
{
	AutoThreadLocker locker(s_locker);
	TComments::iterator it = s_comments.find(id);
	if(it == s_comments.end())
	{
		TSessions sessions;
		sessions.push_back(session);
		s_comments.insert(std::make_pair(id, sessions));
		return true;
	}
	else
	{
		TSessions& sessions =  it->second;
		sessions.push_back(session);
		return false;
	}
}

static bool PopSession(unsigned int id, TSessions& sessions)
{
	AutoThreadLocker locker(s_locker);
	TComments::iterator it = s_comments.find(id);
	assert(it != s_comments.end());
	if(it == s_comments.end())
		return false;

	sessions = it->second;
	s_comments.erase(it);
	return true;
}

static void OnAction(void* param)
{
	libct::auto_ptr<TCommentArgs> args((TCommentArgs*)param);

	std::string comment;
	int r = GetComment(args->id, comment);
	if(0 == r)
	{
		jokecomment_insert(args->id, time64_now(), comment); // update database
	}

	TSessions sessions;
	TSessions::iterator it;
	PopSession(args->id, sessions);
	for(it = sessions.begin(); it != sessions.end(); ++it)
	{
		WebSession* session = *it;
		if(0 != r)
			session->Reply(r, "Get comment failed.");
		else
			session->ReplyArrary("data", comment);
		session->release();
	}
}

int WebSession::OnComment()
{
	int v;
	unsigned int id;
	if(!m_params.Get("id", v))
		return Reply(ERROR_PARAM, "miss id");

	//int content = m_params.Get2("content", 1);

	id = (unsigned int)v;
	time64_t datetime = 0;
	std::string comment;
	int r = jokecomment_query(id, datetime, comment);
	if(0 == r && datetime + 10*60*1000 > time64_now())
		return ReplyArrary("data", comment); // valid if in 10-minutes

	addref();

	// update from website
	if(PushSession(id, this))
	{
		TCommentArgs* args = (TCommentArgs*)malloc(sizeof(TCommentArgs));
		args->id = id;
		r = sys_task_queue_post(g_taskQ, OnAction, args);
		if(0 != r)
		{
			free(args);
			return Reply(r, "post queue error.");
		}
	}
	return 0;
}
