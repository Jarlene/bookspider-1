#include "web-session.h"
#include "joke-db.h"
#include "joke-node.h"
#include "jsonhelper.h"
#include "log.h"
#include <time.h>

static struct rwlist s_texts, s_hot_texts;
static struct rwlist s_images, s_hot_images;

int WebSession::OnLateText()
{
	return OnJoke(&s_texts, "/joke/late-text.php?");
}

int WebSession::OnHotText()
{
	return OnJoke(&s_hot_texts, "/joke/hot-text.php?");
}

int WebSession::OnLateImage()
{
	return OnJoke(&s_images, "/joke/late-image.php?");
}

int WebSession::OnHotImage()
{
	return OnJoke(&s_hot_images, "/joke/hot-image.php?");
}

int WebSession::OnJoke(struct rwlist* list, const char* redirect)
{
	int page = 0;
	//int limit = 50;
	std::string tseq;
	m_params.Get("s", tseq);
	m_params.Get("page", page);
	//m_params.Get("limit", limit);

	std::string json;
	struct joke_node *node;
	node = joke_node_find(list, atoi(tseq.c_str()));
	if(node && page < node->count)
	{
		json = node->jokes[page];
		return Reply(json);
	}
	else
	{
		char uri[128] = {0};
		sprintf(uri, "s=%s&page=%d", redirect, tseq, page);
		return ReplyRedirectTo(uri);
	}
}

static int QueryJokes(struct rwlist* list, int image, int hot)
{
	Jokes jokes;
	int r = jokedb_query_jokes(NULL, image, hot, jokes);
	if(r < 0)
		return r;

	joke_node* node = (joke_node*)malloc(sizeof(joke_node));
	if(!node)
		return -ENOMEM;

	memset(node, 0, sizeof(joke_node));
	node->time = time(NULL);
	node->ref = 1;

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
		json.add("timestamp", (unsigned int)time(NULL));
		json.add("data", jarr[i]);

		node->jokes[node->count++] = strdup(json.json().c_str());
	}

	rwlist_push(list, &node->list);
	return 0;
}

static int OnTextTimer()
{
	int r = QueryJokes(&s_texts, 0, 0);
	log_info("WebSession::OnTextTimer: %d\n", r);
	return r;
}

static int OnHotTextTimer()
{
	int r = QueryJokes(&s_hot_texts, 0, 1);
	log_info("WebSession::OnHotTextTimer: %d\n", r);
	return r;
}

static int OnImageTimer()
{
	int r = QueryJokes(&s_images, 1, 0);
	log_info("WebSession::OnImageTimer: %d\n", r);
	return r;
}

static int OnHotImageTimer()
{
	int r = QueryJokes(&s_hot_images, 1, 1);
	log_info("WebSession::OnHotImageTimer: %d\n", r);
	return r;
}

void WebSession::OnJokeTimer(sys_timer_t /*id*/, void* /*param*/)
{
	OnTextTimer();
	OnImageTimer();
	OnHotTextTimer();
	OnHotImageTimer();
}
