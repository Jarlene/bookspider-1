#include "network-http.h"
#include "url.h"
#include "error.h"
#include "HttpSocket.h"
#include <stdlib.h>
#include <string.h>

int Inflate(const void* ptr, size_t len, mmptr& result);

int network_http(const char* uri, const char* req, mmptr& reply)
{
	assert(uri);

	HttpSocket http;
	http.SetConnTimeout(30*1000); // 30sec(s)
	http.SetRecvTimeout(30*1000); // 30sec(s)
	http.SetHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	http.SetHeader("Accept-Encoding", "gzip, deflate");
	http.SetHeader("Accept-Language", "en-us,en;q=0.5");
	http.SetHeader("Connection", "keep-alive");
	http.SetHeader("User-Agent", "WebSpider 1.0");
	//http.SetHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:14.0) Gecko/20100101 Firefox/14.0.1");

	void* url = url_parse(uri);
	if(!url)
		return ERROR_PARAM;

	int port = url_getport(url);
	int r = http.Connect(url_gethost(url), port?port:80);
	if(r)
	{
		assert(r < 0);
		return r;
	}

	const char* path = url_getpath(url);
	r = req?http.Post(path, req, strlen(req), reply):http.Get(path, reply);
	if(ERROR_HTTP_REDIRECT == r)
	{
	}
	else if(0==r && reply.size()>0)
	{
		std::string contentEncoding;
		http.GetResponse().GetContentEncoding(contentEncoding);
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
	url_free(url);
	return r;
}
