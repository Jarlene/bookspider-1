#include "http-pool.h"
#include "http-proxy.h"
#include "time64.h"
#include "sys-timer.h"
#include "sys/sync.hpp"
#include "libhttp-common.h"
#include <string>
#include <list>
#include <map>

#define TIMEOUT (5*60*1000) // 5-minutes

struct socket_context_t
{
	proxy_object_t* proxy;
	HttpSocket* http;
	host_t host;
	time64_t time;	// 0-using, other-idle
};

typedef std::list<socket_context_t*> TSockets;
typedef std::map<std::string, TSockets> TPool;
static TPool s_pool;
static ThreadLocker s_locker;

static HttpSocket* http_create()
{
	HttpSocket *http = new HttpSocket();

	http->SetConnTimeout(2000);
	http->SetRecvTimeout(10000); // 10sec(s)
	http->SetHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	http->SetHeader("Accept-Encoding", "gzip, deflate");
	http->SetHeader("Accept-Language", "en-us,en;q=0.5");
	http->SetHeader("Connection", "keep-alive");
	http->SetHeader("User-Agent", "WebSpider 1.0");
	//http->SetHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:14.0) Gecko/20100101 Firefox/14.0.1");
	return http;
}

static void http_destroy(socket_context_t* ctx)
{
	if(ctx->proxy)
		http_proxy_release(ctx->proxy);
	if(ctx->http)
		delete ctx->http;
	free(ctx);
}

static void http_pool_ontimer(sys_timer_t id, void* param)
{
	TPool::iterator it;
	TSockets::iterator j;

	time64_t tnow = time64_now();

	AutoThreadLocker locker(s_locker);
	for(it = s_pool.begin(); it != s_pool.end(); ++it)
	{
		TSockets& sockets = it->second;

		for(j = sockets.begin(); j != sockets.end(); ++j)
		{
			socket_context_t* ctx = *j;
			if(0!=ctx->time && ctx->time + TIMEOUT < tnow)
			{
				// release connection
				ctx->http->Disconnect();
			}
		}
	}
}

static socket_context_t* http_pool_get(const std::string& host)
{
	TPool::iterator i;
	TSockets::iterator j;
	socket_context_t* ctx;

	AutoThreadLocker locker(s_locker);
	i  = s_pool.find(host);
	if(i != s_pool.end())
	{
		TSockets& sockets = i->second;
		for(j = sockets.begin(); j != sockets.end(); ++j)
		{
			ctx = *j;
			if(0 != (*j)->time)
			{
				ctx->time = 0;
				return ctx;
			}
		}
	}

	return NULL;
}

static void http_pool_add(socket_context_t* ctx)
{
	TPool::iterator i;
	AutoThreadLocker locker(s_locker);
	i = s_pool.find(ctx->host);
	if(i == s_pool.end())
	{
		i = s_pool.insert(std::make_pair(std::string(ctx->host), TSockets())).first;
	}

	TSockets& sockets = i->second;
	sockets.push_back(ctx);
}

static void http_pool_delete(socket_context_t* ctx)
{
	TPool::iterator i;
	TSockets::iterator j;

	{
		AutoThreadLocker locker(s_locker);
		i = s_pool.find(ctx->host);
		assert(i != s_pool.end());
		if(i == s_pool.end())
			return;

		TSockets& sockets = i->second;
		for(j = sockets.begin(); j != sockets.end(); ++j)
		{
			if(ctx == *j)
			{
				sockets.erase(j);
				break;
			}
		}
	}

	http_destroy(ctx);
}

static socket_context_t* http_pool_find(HttpSocket* http)
{
	TPool::iterator i;
	TSockets::iterator j;

	AutoThreadLocker locker(s_locker);
	for(i = s_pool.begin(); i != s_pool.end(); ++i)
	{
		TSockets& sockets = i->second;
		for(j = sockets.begin(); j != sockets.end(); ++j)
		{
			if(http == (*j)->http)
				return *j;
		}
	}

	return NULL;
}

HttpSocket* http_pool_fetch(const char* host, int port)
{
	int r = 0;
	char id[128] = {0};
	snprintf(id, sizeof(id), "%s:%d", host, port);

	socket_context_t* ctx;
	ctx = http_pool_get(id);
	if(ctx)
	{
		if(ctx->proxy && ctx->proxy->ref < 2)
		{
			// proxy deleted
			http_proxy_release(ctx->proxy);
		}
		else
		{
			if(!ctx->http->IsConnected())
				r = ctx->http->Connect();

			if(0 == r)
				return ctx->http;
		}

		ctx->proxy = NULL;
	}
	else
	{
		ctx = (socket_context_t*)malloc(sizeof(socket_context_t));
		if(!ctx)
			return NULL;
		memset(ctx, 0, sizeof(socket_context_t));

		strcpy(ctx->host, id);
		ctx->http = http_create();
		http_pool_add(ctx);
	}

	// check proxy
	int proxyPort;
	host_t proxyHost;
	proxy_object_t* proxy;
	for(int i=0; i<http_proxy_count(); i++)
	{
		proxy = http_proxy_get(host);
		if(!proxy)
			break;

		host_parse(proxy->proxy, proxyHost, &proxyPort);
		r = ctx->http->Connect(proxyHost, proxyPort);
		if(0 == r)
		{
			ctx->proxy = proxy;
			break;
		}

		http_proxy_release(proxy);
	}

	if(!ctx->proxy)
	{
		// don't use proxy
		r = ctx->http->Connect(host, port);
	}

	return 0==r? ctx->http : NULL;
}

int http_pool_release(HttpSocket* http, int time)
{
	TPool::iterator i;
	TSockets::iterator j;
	socket_context_t* ctx;

	ctx = http_pool_find(http);
	assert(ctx);

#if defined(_DEBUG) || defined(DEBUG)
	int port;
	std::string ip;
	ctx->http->GetHost(ip, port);
	if(ctx->proxy)
	{
		assert(0==strnicmp(ip.c_str(), ctx->proxy->proxy, ip.length()));
		printf("[%s-%s]: %d\n", ctx->host, ctx->proxy->proxy, time);
	}
	else
	{
		assert(0==strnicmp(ip.c_str(), ctx->host, ip.length()));
		printf("[%s]: %d\n", ctx->host, time);
	}
#endif

	if(time < 0)
	{
		http_pool_delete(ctx);
		return 0;
	}
	else
	{
		ctx->time = time64_now();
	}
	return 0;
}

static sys_timer_t timerId;
static int v = sys_timer_start(&timerId, TIMEOUT/2, http_pool_ontimer, NULL);
