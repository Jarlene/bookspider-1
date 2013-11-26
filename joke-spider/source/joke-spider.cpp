#include "JokeSpider.h"
#include "cstringext.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include "tools.h"
#include "web-translate.h"
#include <assert.h>
#include <string>

void escape_nbsp(std::string& src)
{
	std::string::size_type n = 0;
	n = src.find("&nbsp;", n);
	while(std::string::npos != n)
	{
		src.replace(n, 6, 1, ' ');
		n = src.find("&nbsp;", n+1);
	}
}

static int joke_check_helper(void* param, const char* xml)
{
	//XMLParser parser(xml);
	//if(!parser.Valid())
	//	return ERROR_PARAM;

	//std::string content;
	//for(bool j=parser.Foreach("contents/content"); j; j=parser.Next())
	//{
	//	std::string value;
	//	if(parser.GetValue(".", value))
	//	{
	//		if(!content.empty())
	//			content += "\r\n";
	//		content += value;
	//	}
	//}

	//tools_write("e:\\a.xml", xml, strlen(xml));
	return 0;
}

int joke_check(const IJokeSpider* spider, const char* uri, const char* req)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "check-%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("joke_get: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	return web_translate(uri, req, xmlfile.c_str(), joke_check_helper, NULL);
}
