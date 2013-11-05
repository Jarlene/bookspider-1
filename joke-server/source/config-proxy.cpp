#include "http-proxy.h"
#include "cppstringext.h"
#include <string>
#include <stdio.h>
#include <errno.h>

int config_proxy_load()
{
	FILE* fp = fopen("data/proxy.cfg", "r");
	if(!fp)
		return errno;

	char proxy[128] = {0};
	while(fgets(proxy, sizeof(proxy)-1, fp))
	{
		std::string host = Strip(proxy, " \r\n");
		http_proxy_add(host.c_str());
	}

	fclose(fp);
	return 0;
}

static void OnListProxyHelper(void* param, const char* value)
{
	FILE* fp = (FILE*)param;
	fwrite(value, 1, strlen(value), fp);
	fwrite("\r\n", 1, 2, fp);
}

int config_proxy_save()
{
	FILE* fp = fopen("data/proxy.cfg", "w");
	if(!fp)
		return errno;

	http_proxy_list(OnListProxyHelper, fp);

	fclose(fp);
	return 0;
}
