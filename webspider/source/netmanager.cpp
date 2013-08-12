#include "netmanager.h"
#include "http.h"
#include "dxt.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include "tools.h"

static const char* g_http_user_agent = "WebSpider 1.0";

int WebToXml(const char* uri, const char* request, const char* xml, char** result)
{
	void* http = http_open();
	http_set_timeout(http, 30*1000); // 30sec(s)
	http_set_header(http, "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	http_set_header(http, "Accept-Encoding", "gzip, deflate");
	http_set_header(http, "Accept-Language", "en-us,en;q=0.5");
	http_set_header(http, "Connection", "keep-alive");
	http_set_header(http, "User-Agent", g_http_user_agent);
	//http_set_header(http, "User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:14.0) Gecko/20100101 Firefox/14.0.1");
	
	void* reply = NULL;
	int r = request?http_post(http, uri, request, strlen(request), &reply):http_get(http, uri, &reply);
	if(r > 0)
	{
		//tools_write("e:\\a.html", reply, r);
		r = DxTransformHtml(result, (char*)reply, xml);
	}
	else if(ERROR_HTTP_REDIRECT==r)
	{
		*result = (char*)malloc(strlen((char*)reply)+1);
		if(*result)
			strcpy(*result, (char*)reply);
	}

	http_close(http);
	return r;
}
