#ifndef _joke_node_h_
#define _joke_node_h_

#include "sys/sync.h"
#include <time.h>

#define MAX_PAGE 50
#define PAGE_NUM 20
#define PAGE_SIZE 50

struct joke_cache
{
	char *jokes[PAGE_NUM];
	int count;
	long ref;
};

struct joke_node
{
	long locker;
	time_t time;
	struct joke_cache *joke;
};

inline void joke_cache_release(struct joke_cache *joke)
{
	if(0 == InterlockedDecrement(&joke->ref))
		free(joke);
}

inline int joke_node_lock(struct joke_node *node)
{
	while(!atomic_cas(&node->locker, 0, 1))
	{
	}
	return 0;
}

inline int joke_node_unlock(struct joke_node *node)
{
	while(!atomic_cas(&node->locker, 1, 0))
	{
	}
	return 0;
}

inline struct joke_cache* joke_node_fetch(struct joke_node* nodes, int count, time_t t)
{
	int idx;
	struct joke_node *node;
	struct joke_cache *joke;

	joke = NULL;
	idx = t % count;
	node = nodes + idx;

	joke_node_lock(node);
	if(node->time == t)
	{
		joke = node->joke;
		InterlockedIncrement(&joke->ref);
	}
	joke_node_unlock(node);

	return joke;
}

inline int joke_node_push(struct joke_node* nodes, int count, time_t t, struct joke_cache *joke)
{
	int idx;
	struct joke_node *node;
	struct joke_cache *old;

	joke = NULL;
	idx = t % count;
	node = nodes+idx;

	joke_node_lock(node);
	assert(node->time != t);
	old = node->joke;
	node->joke = joke;
	InterlockedIncrement(&joke->ref);
	joke_node_unlock(node);

	joke_cache_release(old);
	return 0;
}

#endif /* !_joke_node_h_ */
