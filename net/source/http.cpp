#include "http.h"
#include "HttpSocket.h"
#include "url.h"
#include "error.h"

struct httpobj
{
	mmptr ptr;
	HttpSocket http;
};

void* http_open()
{
	httpobj* obj = new httpobj();
	return obj;
}

void http_close(void* http)
{
	httpobj* obj = (httpobj*)http;
	if(obj)
		delete obj;
}

void http_set_timeout(void* http, int timeout)
{
	assert(http);
	httpobj* obj = (httpobj*)http;
	obj->http.SetConnTimeout(timeout);
	obj->http.SetRecvTimeout(timeout);
}

int Inflate(const void* ptr, size_t len, mmptr& result);
static int http_request(void* http, const char* uri, const void* req, size_t len, void** reply)
{
	assert(http && uri && reply);

	void* url = url_parse(uri);
	if(!url)
		return -1;

	int port = url_getport(url);
	httpobj* obj = (httpobj*)http;
	int r = obj->http.Connect(url_gethost(url), port?port:80);
	if(r)
	{
		assert(r < 0);
		return r;
	}

	r = req?obj->http.Post(uri, req, len, obj->ptr):obj->http.Get(uri, obj->ptr);
	if(ERROR_HTTP_REDIRECT == r)
	{
		*reply = obj->ptr.get();
	}
	else if(0==r && obj->ptr.size()>0)
	{
		std::string contentEncoding;
		obj->http.GetResponse().GetContentEncoding(contentEncoding);
		if(contentEncoding.size())
		{
			mmptr ptr;
			r = Inflate(obj->ptr.get(), obj->ptr.size(), ptr);
			if(0 == r)
			{
				int n = ptr.size();
				obj->ptr.attach(ptr.detach(), n);
			}
		}

		// auto append '\0'
		obj->ptr.reserve(obj->ptr.size()+1);
		obj->ptr[obj->ptr.size()] = 0;

		*reply = obj->ptr.get();
		return obj->ptr.size();
	}

	assert(r <= 0);
	return r;
}

int http_get(void* http, const char* uri, void** reply)
{
	return http_request(http, uri, NULL, 0, reply);
}

int http_post(void* http, const char* uri, const void* req, unsigned int len, void** reply)
{
	return http_request(http, uri, (!req&&len<1)?"":req, len, reply);
}

int http_set_header(void* http, const char* name, const char* value)
{
	assert(http);
	httpobj* obj = (httpobj*)http;
	obj->http.SetHeader(name, value);
	return 0;
}

int http_get_header(void* http, const char* name, char* value, unsigned int len)
{
	assert(http);
	httpobj* obj = (httpobj*)http;
	const HttpResponse& response = obj->http.GetResponse();
	
	std::string v;
	if(!response.GetHeader(name, v))
		return -1;

	if(len < v.size()+1)
		return v.size()+1;

	strcpy(value, v.c_str());
	return 0;
}
