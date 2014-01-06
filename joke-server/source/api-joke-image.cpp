#include "web-session.h"
#include "joke-db.h"
#include <time.h>

#define MAX_PAGE 50
#define PAGE_NUM 20
#define PAGE_SIZE 50

typedef std::map<std::string, std::vector<std::string> > TJokes;
static TJokes s_jokes;

int WebSession::QueryJokes(const TJokes& jokes)
{
	int page = 0;
	//	int limit = 50;
	std::string tseq;
	m_params.Get("s", tseq);
	m_params.Get("page", page);
	//	m_params.Get("limit", limit);

	TJokes::const_iterator it;
	it = jokes.find(tseq);
	if(it == jokes.end() || page > (int)it->second.size())
	{
		char uri[128] = {0};
		sprintf(uri, "/joke/18plus.php?s=%s&page=%d", tseq, page);
		return ReplyRedirectTo(uri);
	}
	else
	{
		return Reply(it->second[page]);
	}
}

int WebSession::OnHotImage()
{
	return QueryJokes(s_hot_images);
}

int WebSession::OnLateImage()
{
	return QueryJokes(s_images);
}

static int On18PlusTimer()
{
	Jokes jokes;
	int r = jokedb_query_jokes(NULL, 0, 0, jokes);
	if(r < 0)
	{
		log_error("WebSession::On18PlusTimer jokedb_query_comics error: %d\n", r);
		return r;
	}

	std::vector<std::string> rs;
	for(int i = 0; i * PAGE_SIZE < (int)jokes.size() && i < PAGE_NUM; i++)
	{
		jsonarray jarr;
		for(int j = 0; j < PAGE_SIZE && j*PAGE_SIZE < (int)jokes.size(); j++)
		{
			jsonobject jobj;
			jobj.add("id", jokes[i].id);
			jobj.add("icon", jokes[i].icon);
			jobj.add("author", jokes[i].author);
			jobj.add("content", jokes[i].content);
			jobj.add("image", jokes[i].image);
			jobj.add("approve", jokes[i].approve);
			jobj.add("disapprove", jokes[i].disapprove);
			jobj.add("comment", jokes[i].comment);
			jobj.add("datetime", jokes[i].datetime);
			jarr.add(jobj);
		}

		jsonobject json;
		json.add("code", 0).add("msg", "ok");
		json.add("timestamp", (unsigned int)time(NULL));
		json.add("data", jarr);

		rs.push_back(json.json());
	}

	log_info("WebSession::On18PlusTimer ok, comics: %u\n", jokes.size());
	return 0;
}
