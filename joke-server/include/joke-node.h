#ifndef _joke_node_h_
#define _joke_node_h_

#include "joke-list.h"

#define MAX_PAGE 50
#define PAGE_NUM 20
#define PAGE_SIZE 50

#define joke_entry(ptr, type, member) \
	(type*)((char*)ptr-(unsigned long)(&((type*)0)->member))

struct joke_node
{
	struct rwlist list;
	time_t time;
	char *jokes[PAGE_NUM];
	int count;
	long ref;
};

inline joke_node* joke_node_find(struct rwlist* head, time_t t)
{
	struct rwlist *ptr = head->next;

	struct joke_node *node;
	while(ptr)
	{
		node = joke_entry(ptr, joke_node, list);
		if(node->time == t || 0 == t)
			return node;
		ptr = head->next;
	}

	return NULL;
}

inline void joke_node_recycle(struct rwlist* head)
{
}

#endif /* !_joke_node_h_ */
