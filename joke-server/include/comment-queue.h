#ifndef _comment_queue_h_
#define _comment_queue_h_

typedef int (*comment_callback)(void* param, unsigned int id, int code, Comments& comments);

int comment_queue_create();
int comment_queue_destroy();

int comment_queue_post(unsigned int id, comment_callback cb, void* param);

#endif /* !_comment_queue_h_ */
