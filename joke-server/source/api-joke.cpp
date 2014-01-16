#include "web-session.h"
#include "joke-db.h"
#include "jsonhelper.h"
#include "log.h"
#include <time.h>

#define JOKE_NODE_NUM 5

static time_t s_latest;
static struct joke_node s_texts[JOKE_NODE_NUM], s_hot_texts[JOKE_NODE_NUM];
static struct joke_node s_images[JOKE_NODE_NUM], s_hot_images[JOKE_NODE_NUM];

int WebSession::OnLateText()
{
	return OnJoke(s_texts, JOKE_NODE_NUM, "/joke/late-text.php?");
}

int WebSession::OnHotText()
{
	return OnJoke(s_hot_texts, JOKE_NODE_NUM, "/joke/hot-text.php?");
}

int WebSession::OnLateImage()
{
	return OnJoke(s_images, JOKE_NODE_NUM, "/joke/late-image.php?");
}

int WebSession::OnHotImage()
{
	return OnJoke(s_hot_images, JOKE_NODE_NUM, "/joke/hot-image.php?");
}

int WebSession::OnJoke(struct joke_node* list, int count, const char* redirect)
{
	time_t t;
	int page = 0;	
	//int limit = 50;
	std::string tseq;
	m_params.Get("s", tseq);
	m_params.Get("page", page);
	//m_params.Get("limit", limit);

	if(tseq.empty())
		t = s_latest;
	else
		t = atoi(tseq.c_str());

	struct joke_cache *joke;
	joke = joke_node_fetch(list, count, t);
	if(joke && page < joke->count)
	{
		return Reply(joke, page);
	}
	else
	{
		char uri[128] = {0};
		sprintf(uri, "s=%s&page=%d", redirect, tseq, page);
		return ReplyRedirectTo(uri);
	}
}

static int QueryJokes(time_t tnow, struct joke_node* list, int count, int image, int hot)
{
	Jokes jokes;
	int r = jokedb_query_jokes(NULL, image, hot, jokes);
	if(r < 0)
		return r;

	joke_cache* cache = (joke_cache*)malloc(sizeof(joke_cache));
	if(!cache)
		return -ENOMEM;

	memset(cache, 0, sizeof(joke_cache));
	cache->ref = 1;

	jsonarray jarr[PAGE_NUM];
	for(int i = 0; i < (int)jokes.size() && i < PAGE_SIZE*PAGE_NUM; i++)
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
		jarr[i/PAGE_SIZE].add(jobj);
	}

	for(int i = 0; i < (int)(jokes.size()+PAGE_SIZE-1) / PAGE_SIZE && i < PAGE_NUM; i++)
	{
		jsonobject json;
		json.add("code", 0).add("msg", "ok");
		json.add("timestamp", (unsigned int)tnow);
		json.add("data", jarr[i]);

		cache->jokes[cache->count++] = strdup(json.json().c_str());
	}

	joke_node_push(list, count, tnow, cache);
	joke_cache_release(cache);
	return 0;
}

static int OnTextTimer(time_t tnow)
{
	int r = QueryJokes(tnow, s_texts, JOKE_NODE_NUM, 0, 0);
	log_info("WebSession::OnTextTimer: %d\n", r);
	return r;
}

static int OnHotTextTimer(time_t tnow)
{
	int r = QueryJokes(tnow, s_hot_texts, JOKE_NODE_NUM, 0, 1);
	log_info("WebSession::OnHotTextTimer: %d\n", r);
	return r;
}

static int OnImageTimer(time_t tnow)
{
	int r = QueryJokes(tnow, s_images, JOKE_NODE_NUM, 1, 0);
	log_info("WebSession::OnImageTimer: %d\n", r);
	return r;
}

static int OnHotImageTimer(time_t tnow)
{
	int r = QueryJokes(tnow, s_hot_images, JOKE_NODE_NUM, 1, 1);
	log_info("WebSession::OnHotImageTimer: %d\n", r);
	return r;
}

int Query18Plus(time_t tnow);
void WebSession::OnJokeTimer(sys_timer_t /*id*/, void* /*param*/)
{
	s_latest = time(NULL);
	Query18Plus(s_latest);
	OnTextTimer(s_latest);
	OnImageTimer(s_latest);
	OnHotTextTimer(s_latest);
	OnHotImageTimer(s_latest);
	log_info("WebSession::OnJokeTimer\n");
}
