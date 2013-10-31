#include "http-pool.h"
#include "systimer.h"
#include "time64.h"
#include "sys/sync.hpp"
#include <string>
#include <list>
#include <map>

#define MAX_CONNECTION 20

struct pool_context_t
{
	time64_t time;	// 0-using, other-idle
	HttpSocket* http;
};

typedef std::list<pool_context_t> HttpSockets;
typedef std::map<std::string, HttpSockets> TPool;
static TPool s_pool;
static ThreadLocker s_locker;

static HttpSocket* http_create()
{
	HttpSocket *http = new HttpSocket();

	http->SetConnTimeout(30*1000); // 30sec(s)
	http->SetRecvTimeout(30*1000); // 30sec(s)
	http->SetHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	http->SetHeader("Accept-Encoding", "gzip, deflate");
	http->SetHeader("Accept-Language", "en-us,en;q=0.5");
	http->SetHeader("Connection", "keep-alive");
	http->SetHeader("User-Agent", "WebSpider 1.0");
	//http->SetHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:14.0) Gecko/20100101 Firefox/14.0.1");

	return http;
}

static void http_pool_push(HttpSockets& sockets, HttpSocket* http)
{
	pool_context_t item;
	item.http = http;
	item.time = time64_now();
	sockets.push_back(item);
}

static void http_pool_ontimer(systimer_t id, void* param)
{
	TPool::iterator it;
	HttpSockets::iterator j;

	time64_t tnow = time64_now();

	AutoThreadLocker locker(s_locker);
	for(it = s_pool.begin(); it != s_pool.end(); ++it)
	{
		HttpSockets& sockets = it->second;

		j = sockets.begin();
		while(j != sockets.end())
		{
			if(j->time != 0 && j->time + 10*60*1000 < tnow)
			{
				// release connection
				delete j->http;
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

HttpSocket* http_pool_fetch(const char* host, int port)
{
	char id[128] = {0};
	snprintf(id, sizeof(id), "%s.%d", host, port);

	TPool::iterator it;
	HttpSockets::iterator j;

	{
		AutoThreadLocker locker(s_locker);
		it  = s_pool.find(id);
		if(it != s_pool.end())
		{
			HttpSockets& sockets = it->second;
			for(j = sockets.begin(); j != sockets.end(); ++j)
			{
				if(0 != j->time)
				{
					j->time = 0;
					return j->http;
				}
			}
		}
	}

	return http_create();
}

int http_pool_release(HttpSocket* http)
{
	int port;
	std::string host;
	http->GetHost(host, port);

	char id[128] = {0};
	snprintf(id, sizeof(id), "%s.%d", host.c_str(), port);

	{
		AutoThreadLocker locker(s_locker);
		TPool::iterator it = s_pool.find(id);
		if(it == s_pool.end())
		{
			HttpSockets sockets;
			http_pool_push(sockets, http);
			s_pool.insert(std::make_pair(std::string(id), sockets));
			return 0;
		}
		else
		{
			HttpSockets::iterator j;
			HttpSockets& sockets = it->second;
			for(j = sockets.begin(); j != sockets.end(); ++j)
			{
				if(http == j->http)
				{
					assert(0 == j->time);
					j->time = time64_now();
					return 0;
				}
			}

			if(sockets.size() < MAX_CONNECTION)
			{
				http_pool_push(sockets, http);
				return 0;
			}
		}
	}

	delete http; // don't need recycle
	return 0;
}
