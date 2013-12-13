#ifndef _libhttp_common_h_
#define _libhttp_common_h_

#include <string.h>

typedef char host_t[64];

typedef struct _proxy_object_t
{
	host_t proxy;
	int delay;
	int rank;
	long ref;
} proxy_object_t;

proxy_object_t* http_proxy_get(const char* uri);
int http_proxy_release(proxy_object_t* proxy);
void http_proxy_keepalive(); // check proxy alive

inline void host_parse(const char* proxy, host_t host, int *port)
{
	const char* p = strrchr(proxy, ':');
	if(p)
	{
		strncpy(host, proxy, p-proxy);
		host[p-proxy] = '\0';
		*port = atoi(p+1);
	}
	else
	{
		strcpy(host, proxy);
		*port = 80;
	}
}

#endif /* !_libhttp_common_h_ */
