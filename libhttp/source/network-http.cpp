#include "network-http.h"
#include "http-pool.h"
#include "url.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

int Inflate(const void* ptr, size_t len, mmptr& result);

int network_http(const char* uri, const char* req, mmptr& reply)
{
	assert(uri);

	HttpSocket *http;
	
	void* url = url_parse(uri);
	if(!url)
		return ERROR_PARAM;

	int port = url_getport(url);
	http = http_pool_fetch(url_gethost(url), port?port:80);
	if(!http)
		return ERROR_OS_RESOURCE;

	int r = http->Connect("114.80.136.112", 7780);
	if(r)
	{
		assert(r < 0);
		http_pool_release(http);
		return r;
	}

	r = req?http->Post(uri, req, strlen(req), reply):http->Get(uri, reply);
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

	assert(r <= 0);
	http_pool_release(http);
	url_free(url);
	return r;
}
