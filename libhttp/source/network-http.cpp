#include "network-http.h"
#include "sys-timer.h"
#include "http-pool.h"
#include "http-proxy.h"
#include "time64.h"
#include "url.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

int Inflate(const void* ptr, size_t len, mmptr& result);

static int ReqHttp(HttpSocket* http, const char* uri, const char* req, mmptr& reply)
{
	int r = (req&&*req)?http->Post(uri, req, strlen(req)):http->Get(uri);
	if(r < 0)
		return r;

	const HttpResponse& response = http->GetResponse();
	int code = response.GetStatusCode();
	if(code >= 300 && code < 400)
	{
		std::string location;
		if(response.GetHeader("location", location))
			reply.set(location.c_str());
		return ERROR_HTTP_REDIRECT;
	}
	else if(200 != code)
	{
		return -code;
	}

	assert(200 == code);
	reply.set(response.GetReply(), response.GetContentLength());
	if(reply.size()>0)
	{
		std::string contentEncoding;
		response.GetContentEncoding(contentEncoding);
		if(contentEncoding.size())
		{
			mmptr result;
			r = Inflate(reply.get(), reply.size(), result);
			if(0 == r)
			{
				int n = result.size();
				reply.attach(result.detach(), n);
			}
		}

		// auto append '\0'
		reply.reserve(reply.size()+1);
		reply[reply.size()] = 0;
	}

	return r;
}

int network_http(const char* uri, const char* req, mmptr& reply)
{
	assert(uri);

	void* url = url_parse(uri);
	if(!url)
		return ERROR_PARAM;

	int port = url_getport(url);
	const char* host = url_gethost(url);
	if(!host)
		return ERROR_PARAM; // url: http://www.host.com/path?param

	HttpSocket *http = NULL;
	http = http_pool_fetch(host, port?port:80);
	if(!http)
	{
		url_free(url);
		return ERROR_OS_RESOURCE;
	}

	time64_t t0 = time64_now();
	int r = ReqHttp(http, uri, req, reply);
	time64_t t1 = time64_now();
	assert(r <= 0);
	http_pool_release(http, r<0?-1:int(t1-t0));
	url_free(url);
	return r;
}

//static void http_timer(sys_timer_t id, void* param)
//{
//	http_proxy_keepalive();
//}
//
//static sys_timer_t timerId;
//static int v = sys_timer_start(&timerId, 30*60*1000, http_timer, NULL);

static void http_timer(sys_timer_t id, void* param)
{
	http_pool_gc();
}

static sys_timer_t timerId;
static int v = sys_timer_start(&timerId, HTTP_POOL_TIMEOUT/2, http_timer, NULL);
