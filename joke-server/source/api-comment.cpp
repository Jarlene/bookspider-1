#include "web-session.h"
#include "joke-comment.h"
#include "comment-queue.h"
#include "JokeSpider.h"
#include "QiuShiBaiKe.h"
#include "BaiSiBuDeJie.h"

static void PostComment(void* param, unsigned int id, int code, const std::string& comment)
{
	WebSession* session = (WebSession*)param;
	if(0 != code)
	{
		session->Reply(code);
	}
	else
	{
		jokecomment_insert(id, time64_now(), comment); // update database
		session->Reply(0, comment);
	}
}

void WebSession::OnComment()
{
	unsigned int id;
	if(!m_params.Get("id", (int&)id))
		return Reply(ERROR_PARAM, "miss id");

	//int content = m_params.Get2("content", 1);

	time64_t datetime;
	std::string comment;
	int r = jokecomment_query(id, datetime, comment);
	if(0 == r && datetime + 10*60 > time64_now())
	{
		Reply(0, comment); // valid if in 10-minutes
	}
	else
	{
		// update from website
		r = comment_queue_post(id, PostComment, this);
		if(0 != r)
			Reply(r, "post queue error.");
	}
}
