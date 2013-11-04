#include "http-proxy.h"
#include "cstringext.h"
#include "sys/sock.h"
#include "sys/sync.hpp"
#include "sys-timer.h"
#include "time64.h"
#include "error.h"
#include "libhttp-common.h"
#include <string>
#include <vector>
#include <list>
#include <algorithm>

#define MAX_PROXY 100
#define TIMEOUT 5000

typedef struct _proxy_object_t
{
	host_t proxy;
	int delay;
	int rank;
	int ref;
} proxy_object_t;

typedef std::list<proxy_object_t> TProxies;
typedef std::list<std::string> TPatterns;

static ThreadLocker s_locker;
static TProxies s_proxies; // proxy set
static TPatterns s_patterns;
static TPatterns s_whitelist;

static int check_pattern(const char* value, const char* pattern)
{
	if('*' == pattern[0])
	{
		assert('.' == pattern[1]);
		return 0 != strstr(value, pattern+1);
	}
	return 0 == stricmp(value, pattern);
}

int http_proxy_add(const char* proxy)
{
	proxy_object_t obj;
	memset(&obj, 0, sizeof(obj));
	strncpy(obj.proxy, proxy, sizeof(obj.proxy)-1);
	obj.delay = 0; // not sure
	obj.rank = 0; // normal

	AutoThreadLocker locker(s_locker);
	if(s_proxies.size() >= MAX_PROXY)
		return ERROR_PARAM;

	s_proxies.push_back(obj);
	return 0;
}

int http_proxy_delete(const char* proxy)
{
	TProxies::iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
	{
		if(0 == stricmp(proxy, it->proxy))
		{
			s_proxies.erase(it);
			return 0;
		}
	}

	return ERROR_NOTFOUND;
}

int http_proxy_list(http_proxy_proc proc, void* param)
{
	TProxies::iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
		proc(param, it->proxy);
	return 0;
}

static int http_proxy_check_pattern(const char* host)
{
	TPatterns::const_iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_patterns.begin(); it != s_patterns.end(); ++it)
	{
		if(1 == check_pattern(host, it->c_str()))
			return 1;
	}
	return 0;
}

int http_proxy_add_pattern(const char* pattern)
{
	AutoThreadLocker locker(s_locker);
	s_patterns.push_back(std::string(pattern));
	return 0;
}

int http_proxy_delete_pattern(const char* pattern)
{
	TPatterns::iterator it;
	AutoThreadLocker locker(s_locker);
	it = std::find(s_patterns.begin(), s_patterns.end(), std::string(pattern));
	if(it == s_patterns.end())
		return ERROR_NOTFOUND;
	s_patterns.erase(it);
	return 0;
}

int http_proxy_list_pattern(http_proxy_proc proc, void* param)
{
	TPatterns::const_iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_patterns.begin(); it != s_patterns.end(); ++it)
		proc(param, it->c_str()); // can't delete pattern in callback
	return 0;
}

static int http_proxy_check_allow_pattern(const char* host)
{
	TPatterns::const_iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_whitelist.begin(); it != s_whitelist.end(); ++it)
	{
		if(1 == check_pattern(host, it->c_str()))
			return 1;
	}
	return 0;
}

int http_proxy_add_allow_pattern(const char* pattern)
{
	AutoThreadLocker locker(s_locker);
	s_whitelist.push_back(std::string(pattern));
	return 0;
}

int http_proxy_delete_allow_pattern(const char* pattern)
{
	TPatterns::iterator it;
	AutoThreadLocker locker(s_locker);
	it = std::find(s_whitelist.begin(), s_whitelist.end(), std::string(pattern));
	if(it == s_whitelist.end())
		return ERROR_NOTFOUND;
	s_whitelist.erase(it);
	return 0;
}

int http_proxy_list_allow_pattern(http_proxy_proc proc, void* param)
{
	TPatterns::const_iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_whitelist.begin(); it != s_whitelist.end(); ++it)
		proc(param, it->c_str()); // can't delete pattern in callback
	return 0;
}

static int http_proxy_set_delay(const host_t proxy, int delay)
{
	TProxies::iterator it;
	AutoThreadLocker locker(s_locker);
	for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
	{
		if(0 == stricmp(it->proxy, proxy))
		{
			it->rank = delay > 0 ? 0 : (it->rank-1);
			it->delay = delay;
			return 0;
		}
	}
	return ERROR_NOTFOUND;
}

struct TProxyKeepAlive
{
	socket_t socket;
	host_t proxy;
	int delay;
};

#if defined(OS_LINUX)
static void http_proxy_keepalive(sys_timer_t id, void* param)
{
	std::vector<TProxyKeepAlive> proxies;
	{
		TProxies::iterator it;
		AutoThreadLocker locker(s_locker);
		for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
		{
			TProxyKeepAlive proxy;
			memset(&proxy, 0, sizeof(TProxyKeepAlive));
			strcpy(proxy.proxy, it->proxy);
			proxies.push_back(proxy);
		}
	}

	int r, port;
	host_t host;
	struct pollfd fds[MAX_PROXY];

	for(int i=(int)proxies.size()-1; i>=0; i--)
	{
		host_parse(proxies[i].proxy, host, &port);

		proxies[i].socket = socket_tcp();
		r = socket_setnonblock(proxies[i].socket, 1);
		r = socket_connect_ipv4(proxies[i].socket, host, (unsigned short)port);
		assert(r <= 0);
		if(0==r || EINPROGRESS==errno)
		{
		}
		else
		{
			socket_close(proxies[i].socket);
			proxies[i].socket = 0;
			http_proxy_set_delay(proxies[i].proxy, -1);
			proxies.erase(proxies.begin()+i);
		}
	}

	time64_t t0 = time64_now();
	time64_t tnow = t0;
	do
	{
		memset(fds, 0, sizeof(fds));
		for(int i=(int)proxies.size()-1; i>=0; i--)
		{
			fds[i].fd = proxies[i].socket;
			fds[i].events = POLLOUT;
			fds[i].revents = 0;
		}

		if(t0 + TIMEOUT <= tnow)
			break;

		int timeout = TIMEOUT-(int)(tnow - t0);
		r = poll(fds, proxies.size(), timeout);

		tnow = time64_now();
		for(int i=(int)proxies.size()-1; i>=0; i--)
		{
			if(POLLOUT & fds[i].revents)
			{
				socket_close(proxies[i].socket);
				http_proxy_set_delay(proxies[i].proxy, (int)(tnow - t0));
				proxies.erase(proxies.begin()+i);
			}
		}
	} while(r > 0 && proxies.size() > 0);

	for(size_t i = 0; i < proxies.size(); i++)
	{
		socket_close(proxies[i].socket);
		http_proxy_set_delay(proxies[i].proxy, -1);
	}
}

#else

static void http_proxy_keepalive(sys_timer_t id, void* param)
{
	std::vector<TProxyKeepAlive> proxies;
	{
		TProxies::iterator it;
		AutoThreadLocker locker(s_locker);
		for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
		{
			TProxyKeepAlive proxy;
			memset(&proxy, 0, sizeof(TProxyKeepAlive));
			strcpy(proxy.proxy, it->proxy);
			proxies.push_back(proxy);
		}
	}

	int r, port;
	host_t host;
	for(size_t i=0; i<proxies.size(); i++)
	{
		host_parse(proxies[i].proxy, host, &port);

		proxies[i].socket = socket_tcp();
		r = socket_setnonblock(proxies[i].socket, 1);
		r = socket_connect_ipv4(proxies[i].socket, host, (unsigned short)port);
		assert(r <= 0);
		if(0==r || WSAEWOULDBLOCK==WSAGetLastError())
		{
			//FD_SET(sockets[i], &fds);
		}
		else
		{
			socket_close(proxies[i].socket);
			proxies[i].socket = 0;
		}

		proxies[i].delay = -1;
	}

	time64_t t0 = time64_now();
	time64_t tnow = t0;
	do
	{
		fd_set fds;
		FD_ZERO(&fds);
		socket_t socket = 0;
		for(size_t i=0; i<proxies.size(); i++)
		{
			socket = max(proxies[i].socket, socket);
			if(0 != proxies[i].socket)
				FD_SET(proxies[i].socket, &fds);
		}

		int timeout = TIMEOUT-(int)(tnow - t0);

		struct timeval tv;
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000) * 1000;
		r = socket_select_writefds(socket+1, &fds, &tv);

		tnow = time64_now();
		for(size_t i = 0; i < proxies.size(); i++)
		{
			if(FD_ISSET(proxies[i].socket, &fds))
				proxies[i].delay = (int)(tnow - t0);
		}
	} while(r > 0 && t0 + TIMEOUT <= tnow);

	printf("==============Proxy Keep-Alive==============\n");
	for(size_t i = 0; i < proxies.size(); i++)
	{
		if(proxies[i].delay >= 0)
			printf("%s - %d\n", proxies[i].proxy, proxies[i].delay);

		if(0 != proxies[i].socket)
		{
			socket_close(proxies[i].socket);
			http_proxy_set_delay(proxies[i].proxy, proxies[i].delay);
		}
	}
}

#endif

int http_proxy_get(const char* uri, host_t proxy)
{
	static int s_idx = 0;

	TProxies::iterator it;
	AutoThreadLocker locker(s_locker);
	if(1 == http_proxy_check_pattern(uri) 
		&& 0 == http_proxy_check_allow_pattern(uri))
	{
		// need proxy
		TProxies::iterator it;

		// delay
		std::vector<TProxies::iterator> iters;
		for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
		{
			if(it->delay > 0)
				iters.push_back(it);
		}

		if(iters.empty())
		{
			for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
				iters.push_back(it);
		}

		if(iters.empty())
			return -1;

		it = iters[(s_idx++) % iters.size()];
		strcpy(proxy, it->proxy);
		++it->ref;
		return 0;
	}

	// don't need proxy
	return -1;
}

int http_proxy_release(host_t proxy)
{
	TProxies::iterator it;
	for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
	{
		if(0 == stricmp(it->proxy, proxy))
		{
			--it->rank;
			return 0;
		}
	}
	return ERROR_NOTFOUND;
}

static sys_timer_t timerId;
static int v = sys_timer_start(&timerId, 30*60*1000, http_proxy_keepalive, NULL);
