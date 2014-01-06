#include "web-session.h"
#include "joke-db.h"
#include "jsonhelper.h"
#include "log.h"
#include <time.h>

#define MAX_PAGE 50
#define PAGE_NUM 20
#define PAGE_SIZE 50

struct rwlist
{
	struct rwlist *prev;
	struct rwlist *next;
};

struct joke_node
{
	struct rwlist list;
	char *jokes[5];
};

static struct rwlist s_comics, s_hot_images, s_hot_texts, s_images, s_texts;

#if defined(OS_WINDOWS)
	#define atomic_cas(d, c, s) (node==InterlockedCompareExchange(&list->prev, node, node->prev))
#else
	#define atomic_cas(d, c, s) __sync_bool_compare_and_swap(&list->prev, node, node->prev)
#endif

void rwlist_push(struct rwlist *head, struct rwlist* node)
{
	struct rwlist *tail;
	node->next = list;

	do
	{
		tail = list->prev;
		node->prev = tail;
	} while(atomic_cas(&list->prev, tail, node));

	atomic_cas(&list->prev->next = node);
}

void rwlist_pop(struct rwlist *head)
{
	struct rwlist *node;
	node = list->prev;

	atomic_cas(&list->prev, node, node->prev);
}

int WebSession::On18Plus()
{
	int page = 0;
//	int limit = 50;
	std::string tseq;
	m_params.Get("s", tseq);
	m_params.Get("page", page);
//	m_params.Get("limit", limit);

	std::string json;
	TComics::const_iterator it;
	if(!tseq.empty())
	{
		it = std::find(s_comics.begin(), s_comics.end(), tseq);
		if(it == s_comics.end() || page > (int)it->second.size())
		{
			char uri[128] = {0};
			sprintf(uri, "/joke/18plus.php?s=%s&page=%d", tseq, page);
			return ReplyRedirectTo(uri);
		}
	}
	else
	{
		it = s_comics.begin();
	}

	json = it->second[page];
	return Reply(json);
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
	for(int i = 0; i * PAGE_SIZE < (int)comics.size() && i < PAGE_NUM; i++)
	{
		jsonarray jarr;
		for(int j = 0; j < PAGE_SIZE && j*PAGE_SIZE < (int)comics.size(); j++)
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
		json.add("timestamp", (unsigned int)time(NULL));
		json.add("data", jarr);

		rs.push_back(json.json());
	}

	log_info("WebSession::On18PlusTimer ok, comics: %u\n", comics.size());
	return 0;
}
