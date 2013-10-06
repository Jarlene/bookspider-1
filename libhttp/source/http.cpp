#include "http.h"
#include "dxt.h"
#include "url.h"
#include "HttpSocket.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

int Inflate(const void* ptr, size_t len, mmptr& result);

int http_request(const char* uri, const char* req, void** reply)
{
	assert(uri && reply);

	mmptr ptr;
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
		return -1;

	int port = url_getport(url);
	int r = http.Connect(url_gethost(url), port?port:80);
	if(r)
	{
		assert(r < 0);
		return r;
	}

	r = req?http.Post(uri, req, strlen(req), ptr):http.Get(uri, ptr);
	if(ERROR_HTTP_REDIRECT == r)
	{
		*reply = ptr.detach();
	}
	else if(0==r && ptr.size()>0)
	{
		std::string contentEncoding;
		http.GetResponse().GetContentEncoding(contentEncoding);
		if(contentEncoding.size())
		{
			mmptr result;
			r = Inflate(ptr.get(), ptr.size(), result);
			if(0 == r)
			{
				int n = result.size();
				ptr.attach(result.detach(), n);
			}
		}

		// auto append '\0'
		ptr.reserve(ptr.size()+1);
		ptr[ptr.size()] = 0;

		*reply = ptr.detach();
	}

	assert(r <= 0);
	url_free(url);
	return r;
}
