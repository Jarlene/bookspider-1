#ifndef _http_pool_
#define _http_pool_

#include "HttpSocket.h"

HttpSocket* http_pool_fetch(const char* host, int port);

int http_pool_release(HttpSocket* http);

#endif /* !_http_pool_ */
