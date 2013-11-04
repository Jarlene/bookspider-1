#ifndef _libhttp_common_h_
#define _libhttp_common_h_

#include <string.h>

typedef char host_t[64];

int http_proxy_get(const char* uri, host_t proxy);
int http_proxy_release(host_t proxy);

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
