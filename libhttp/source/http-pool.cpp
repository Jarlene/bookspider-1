#include "http-pool.h"
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
	HttpSocket* http;
	host_t host;
	host_t proxy;
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
	http->SetRecvTimeout(30*1000); // 30sec(s)
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
	http_proxy_release(ctx->proxy);
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

		j = sockets.begin();
		while(j != sockets.end())
		{
			socket_context_t* ctx = *j;
			assert(0 != ctx->time);
			if(ctx->time + TIMEOUT < tnow)
			{
				// release connection
				http_destroy(ctx);
				sockets.erase(j);
				j = sockets.begin();
			}
			else
			{
				++j;
			}
		}
	}
}

static socket_context_t* http_pool_pop(const std::string& host)
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
				sockets.erase(j); // delete from sockets
				return ctx;
			}
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
	ctx = http_pool_pop(id);
	if(ctx)
	{
		if(!ctx->http->IsConnected())
			r = ctx->http->Connect();

		if(0 == r)
			return ctx->http;

		strcpy(ctx->proxy, "");
	}
	else
	{
		ctx = (socket_context_t*)malloc(sizeof(socket_context_t));
		if(!ctx)
			return NULL;
		memset(ctx, 0, sizeof(socket_context_t));

		strcpy(ctx->host, id);
		ctx->http = http_create();
	}
	
	// check proxy
	int proxyPort;
	host_t proxyHost;
	host_t proxy = {0};
	for(int i=0; i<10 && 0==http_proxy_get(host, proxy); i++)
	{
		host_parse(proxy, proxyHost, &proxyPort);
		r = ctx->http->Connect(proxyHost, proxyPort);
		if(0 == r)
		{
			strcpy(ctx->proxy, proxy);
			break;
		}

		http_proxy_release(proxy);
	}

	if(0 != ctx->proxy[0])
	{
		// don't use proxy
		r = ctx->http->Connect(host, port);
	}

	if(0 != r)
	{
		http_destroy(ctx);
		return NULL;
	}
	return ctx->http;
}

int http_pool_release(HttpSocket* http, int time)
{
	TPool::iterator i;
	TSockets::iterator j;
	socket_context_t* ctx;

	ctx = (socket_context_t*)((char*)http - (unsigned long)(&((socket_context_t*)0)->http));
	if(-1 == time)
	{
		http_destroy(ctx);
		return 0;
	}

	ctx->time = time64_now();

	AutoThreadLocker locker(s_locker);
	i = s_pool.find(ctx->host);
	if(i == s_pool.end())
	{
		i = s_pool.insert(std::make_pair(std::string(ctx->host), TSockets())).first;
	}

	TSockets& sockets = i->second;
	sockets.push_back(ctx); // new
	return 0;
}

static sys_timer_t timerId;
static int v = sys_timer_start(&timerId, TIMEOUT/2, http_pool_ontimer, NULL);
