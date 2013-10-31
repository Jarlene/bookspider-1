#ifndef _http_proxy_h_
#define _http_proxy_h_

typedef void (*http_proxy_onfind)(void* param, const char* ip, int port);

int http_proxy_find(http_proxy_onfind callback, void* param);

#endif /* !_http_proxy_h_ */
