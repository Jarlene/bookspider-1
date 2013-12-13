#ifndef _http_pool_
#define _http_pool_

#include "HttpSocket.h"

#define HTTP_POOL_TIMEOUT (5*60*1000) // 5-minutes

HttpSocket* http_pool_fetch(const char* host, int port);

int http_pool_release(HttpSocket* http, int time);

void http_pool_gc();

#endif /* !_http_pool_ */
