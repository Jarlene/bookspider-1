#include "web-session.h"
#include "joke-db.h"
#include "joke-node.h"
#include "jsonhelper.h"
#include "log.h"
#include <time.h>

static struct rwlist s_comics;

int WebSession::On18Plus()
{
	return OnJoke(&s_comics, "/joke/18plus.php?");
}

static int Query18Plus()
{
	Comics comics;
	int r = jokedb_query_comics(NULL, comics);
	if(r < 0)
	{
		log_error("WebSession::On18PlusTimer jokedb_query_comics error: %d\n", r);
		return r;
	}

	joke_node* node = (joke_node*)malloc(sizeof(joke_node));
	if(!node)
		return -ENOMEM;

	memset(node, 0, sizeof(joke_node));
	node->time = time(NULL);
	node->ref = 1;

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
		json.add("timestamp", (unsigned int)time(NULL));
		json.add("data", jarr[i]);

		node->jokes[node->count++] = strdup(json.json().c_str());
	}

	rwlist_push(&s_comics, &node->list);
	log_info("WebSession::On18PlusTimer ok, comics: %u\n", comics.size());
	return 0;
}

void WebSession::On18PlusTimer(sys_timer_t /*id*/, void* /*param*/)
{
	Query18Plus();
}
