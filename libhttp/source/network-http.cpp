#include "network-http.h"
#include "http-pool.h"
#include "http-proxy.h"
#include "url.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

int Inflate(const void* ptr, size_t len, mmptr& result);

static int ReqHttp(HttpSocket* http, const char* uri, const char* req, mmptr& reply)
{
	int r = (req&&*req)?http->Post(uri, req, strlen(req), reply):http->Get(uri, reply);
	if(ERROR_HTTP_REDIRECT == r)
	{
	}
	else if(0==r && reply.size()>0)
	{
		std::string contentEncoding;
		http->GetResponse().GetContentEncoding(contentEncoding);
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

	HttpSocket *http = NULL;
	http = http_pool_fetch(host, port?port:80);
	if(!http)
	{
		url_free(url);
		return ERROR_OS_RESOURCE;
	}

	int r = ReqHttp(http, uri, req, reply);
	assert(r <= 0);
	http_pool_release(http, r);
	url_free(url);
	return r;
}
