#include "http-proxy.h"
#include "cstringext.h"
#include "http-translate.h"
#include "http.h"
#include "XMLParser.h"

static int http_proxy_check(const char* ip, int port)
{
	void* reply;
	char uri[256] = {0};
	snprintf(uri, sizeof(uri), "http://%s:%d/", ip, port);
	return http_request(uri, "", &reply);
}

typedef struct
{
	http_proxy_onfind callback;
	void* param;
} TFindParam;

static int OnFindProxy(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	TFindParam* p = (TFindParam*)param;
	for(bool i=parser.Foreach("proxy"); i; i=parser.Next())
	{
		int port = 0;
		std::string ip, country, transparent, https;
		if(!parser.GetValue("ip", ip) || !parser.GetValue("port", port))
			continue;

		// check valid
		if(ip.empty() || 0==port)
			continue;

		parser.GetValue("country", country);
		parser.GetValue("transparent", transparent);
		parser.GetValue("https", https);

		if(0 == http_proxy_check(ip.c_str(), port))
			p->callback(p->param, ip.c_str(), port);
	}

	return 0;
}

int http_proxy_find(http_proxy_onfind callback, void* param)
{
	TFindParam p = {callback, param};

	int r = 0;
	char uri[256] = {0};
	for(int i=1; i<=8; i++)
	{
		snprintf(uri, sizeof(uri), "http://www.cnproxy.com/proxy%d.html", i);
		r = http_translate(uri, NULL, "data/proxyhttp.xml", OnFindProxy, &p);
		if(0 != r)
			break;
	}

	return r;
}
