#include "web-session.h"
#include "joke-db.h"
#include "jsonhelper.h"
#include "log.h"

#define MAX_PAGE 50
#define PAGE_NUM 20
#define PAGE_SIZE 50

typedef std::map<std::string, std::vector<std::string> > TComics;
static TComics s_comics;

int WebSession::On18Plus()
{
	int page = 0;
	int limit = 50;
	std::string tseq;
	m_params.Get("s", tseq);
	m_params.Get("page", page);
//	m_params.Get("limit", limit);

	TComics::const_iterator it;
	it = s_comics.find(tseq);
	if(it == s_comics.end() || page > it->second.size())
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

static int On18PlusTimer()
{
	Comics comics;
	int r = jokedb_query_comics(NULL, comics);
	if(r < 0)
	{
		log_error("WebSession::On18PlusTimer jokedb_query_comics error: %d\n", r);
		return r;
	}

	std::vector<std::string> rs;
	for(int i = 0; i * PAGE_SIZE < comics.size() && i < PAGE_NUM; i++)
	{
		jsonarray jarr;
		for(int j = 0; j < PAGE_SIZE && j*PAGE_SIZE < comics.size(); j++)
		{
			jsonobject jobj;
			jobj.add("id", comics[i].id);
			jobj.add("title", comics[i].title);
			jobj.add("text", comics[i].text);
			jobj.add("image", comics[i].images.size()>0 ? comics[i].images.front().c_str() : "");
			jobj.add("datetime", comics[i].datetime);
			jarr.add(jobj);
		}

		jsonobject json;
		json.add("code", 0).add("msg", "ok");
		json.add("timestamp", t);
		json.add("data", jarr);

		rs.push_back(json.json());
	}

	log_info("WebSession::On18PlusTimer ok, comics: %u\n", comics.size());
	return 0;
}
