#ifndef _joke_list_h_
#define _joke_list_h_

#include "sys/sync.h"

struct rwlist
{
	rwlist *next;
};

inline void rwlist_push(struct rwlist *head, struct rwlist* node)
{
	struct rwlist *p;

	do
	{
		p = head->next;
		node->next = p;
	} while(!atomic_cas((long*)&(head->next), (long)p, (long)node));
}

inline void rwlist_pop(struct rwlist *head)
{
	struct rwlist *node;
	struct rwlist *next;

	do
	{
		if(head->next == NULL)
			return;

		node = head;
		next = head->next;
		while(next->next)
		{
			node = next;
			next = next->next;
		}
	} while(!atomic_cas((long*)&node->next, (long)next, (long)NULL));
}

#endif /* !_joke_list_h_ */
