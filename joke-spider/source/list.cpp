#include "JokeSpider.h"
#include "cstringext.h"
#include "sys/system.h"
#include "libct/auto_ptr.h"
#include "http.h"
#include "dxt.h"
#include "XMLParser.h"
#include "utf8codec.h"
#include "error.h"
#include "config.h"
#include "tools.h"
#include <assert.h>
#include <string>

static int Parse(const char* xml, OnJoke callback, void* param)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	for(bool i=parser.Foreach("jokes/joke"); i; i=parser.Next())
	{
		int approve = 0;
		int disapprove = 0;
		std::string id, author, datetime, content, image;
		if(!parser.GetValue("id", id)
			|| !parser.GetValue("content", content)
			|| !parser.GetValue("approve", approve)
			|| !parser.GetValue("disapprove", disapprove))
			continue;

		// check valid
		if(id.empty() || content.empty())
			continue;

		parser.GetValue("image", image);
		parser.GetValue("author", author);
		parser.GetValue("datetime", datetime);

		// 13-03-07 21:03 => 2013-03-07 21:03
		if(datetime.length() == 14)
			datetime.insert(0, "20");
		else if(datetime.length() == 11)
			datetime.insert(0, "2013-");

		// to utf-8
		const char* encoding = parser.GetEncoding();
		callback(param, UTF8Encode(id.c_str(), encoding), 
			UTF8Encode(author.c_str(), encoding), 
			UTF8Encode(datetime.c_str(), encoding),
			UTF8Encode(content.c_str(), encoding),
			UTF8Encode(image.c_str(), encoding),
			approve, disapprove);
	}

	return 0;
}

static int Http(const char* uri, const char* req, void** reply)
{
	int r = -1;
	for(int i=0; r<0 && i<20; i++)
	{
		r = http_request(uri, req, reply);
		if(r < 0)
		{
			printf("get %s error: %d\n", uri, r);
			system_sleep(10);
		}
	}
	return r;
}

static int s_i = 1;
int ListJoke(const IJokeSpider* spider, const char* uri, const char* req, OnJoke callback, void* param)
{
	char name[64] = {0};
	std::string xmlfile;
	sprintf(name, "web-%s", spider->GetName());
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("ListJoke: can't find %s xml file.\n", spider->GetName());
		return ERROR_NOTFOUND;
	}

	libct::auto_ptr<char> reply;
	int r = Http(uri, req, (void**)&reply);
	if(r < 0)
	{
		printf("Check update error: %d\n", r);
		return r;
	}

	libct::auto_ptr<char> result;
	r = DxTransformHtml(&result, reply, xmlfile.c_str());
	if(r < 0)
	{
		tools_write("e:\\a.html", reply, strlen(reply));
		printf("ListJoke[%s]: error: %d.\n", spider->GetName(), r);
		return r;
	}

	return Parse(result, callback, param);
}
