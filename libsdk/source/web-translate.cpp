#include "web-translate.h"
#include "cstringext.h"
#include "sys/system.h"
#include "libct/auto_ptr.h"
#include "tools.h"
#include "error.h"
#include "dxt.h"
#include "http.h"
#include "mmptr.h"
#include "url.h"
#include <stdio.h>
#include <assert.h>

static void uri_format(char* uri, const char* from, const char* to)
{
	void* toUri = url_parse(to);
	assert(toUri);

	if(url_gethost(toUri))
	{
		strcpy(uri, to);
	}
	else
	{
		void* fromUri = url_parse(from);
		assert(fromUri && url_gethost(fromUri));

		int port = url_getport(fromUri);
		sprintf(uri, "%s://%s:%d%s", url_getscheme(fromUri), url_gethost(fromUri), 0==port?80:port, to);

		url_free(fromUri);
	}

	url_free(toUri);
}

static int http(const char* uri, const char* req, mmptr& reply)
{
	int r = -1;
	char newUri[256] = {0};
	for(int i=0; r<0 && i<20; i++)
	{
		int len = 0;
		void* response = NULL;
		r = http_request(uri, req, &response, &len);
		if(ERROR_HTTP_REDIRECT == r)
		{
			assert(response && len > 0);
			uri_format(newUri, uri, (char*)response);
			printf("Http Redirect:\n[From]: %s\n[To]:%s\n[URI]:%s\n", uri, (char*)response, newUri);
			free(response);
			response = NULL;
			len = 0;
			r = http_request(newUri, req, &response, &len);
		}

		if(r < 0)
		{
			assert(NULL == response && 0 == len);
			printf("get %s error: %d\n", uri, r);
			system_sleep(5000);
		}
		else
		{
			reply.attach(response, len);
			return 0;
		}
	}
	return r;
}

int web_translate(const char* uri, const char* req, const char* xml, OnTranslated callback, void* param)
{
	mmptr reply;
	int r = http(uri, req, reply);
	if(r < 0)
	{
		printf("http_translate [get]%s: %d\n", uri, r);
		return r;
	}

	libct::auto_ptr<char> result;
	r = DxTransformHtml(&result, reply, xml);
	if(r < 0)
	{
		//tools_write("e:\\a.html", reply, strlen(reply));
		printf("http_translate [dxt]%s: %d\n", uri, r);
		return r;
	}

	return callback ? callback(param, result) : ERROR_PARAM;
}
