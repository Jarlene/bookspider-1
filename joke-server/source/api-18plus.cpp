#include "web-session.h"
#include "joke-db.h"
#include "jsonhelper.h"
#include "log.h"
#include <time.h>

#define JOKE_NODE_NUM 5

static struct joke_node s_comics[JOKE_NODE_NUM];

int WebSession::On18Plus()
{
	return OnJoke(s_comics, JOKE_NODE_NUM, "/joke/18plus.php?");
}

int Query18Plus(time_t tnow)
{
	Comics comics;
	int r = jokedb_query_comics(NULL, comics);
	if(r < 0)
	{
		log_error("Query18Plus jokedb_query_comics error: %d\n", r);
		return r;
	}

	joke_cache* cache = (joke_cache*)malloc(sizeof(joke_cache));
	if(!cache)
		return -ENOMEM;

	memset(cache, 0, sizeof(joke_cache));
	cache->ref = 1;

	jsonarray jarr[PAGE_NUM];
	for(int i = 0; i < (int)comics.size() && i < PAGE_SIZE*PAGE_NUM; i++)
	{
		jsonobject jobj;
		jobj.add("id", comics[i].id);
		jobj.add("title", comics[i].title);
		jobj.add("text", comics[i].text);
		jobj.add("image", comics[i].images.size()>0 ? comics[i].images.front().c_str() : "");
		jobj.add("datetime", comics[i].datetime);
		jarr[i/PAGE_SIZE].add(jobj);
	}

	for(int i = 0; i < (int)(comics.size()+PAGE_SIZE-1) / PAGE_SIZE && i < PAGE_NUM; i++)
	{
		jsonobject json;
		json.add("code", 0).add("msg", "ok");
		json.add("timestamp", (unsigned int)tnow);
		json.add("data", jarr[i]);

		cache->jokes[cache->count++] = strdup(json.json().c_str());
	}

	joke_node_push(s_comics, JOKE_NODE_NUM, tnow, cache);
	joke_cache_release(cache);
	return 0;
}
