#include "YaoYao.h"
#include "cstringext.h"
#include "sys/system.h"
#include "error.h"
#include "config.h"
#include "XMLParser.h"
#include "web-translate.h"

static int image_parser(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	Comic comic;
	if(!parser.GetValue("title", comic.title)
		|| !parser.GetValue("datetime", comic.datetime))
		return ERROR_PARAM;

	size_t n = comic.title.find('-');
	if(std::string::npos != n)
		comic.title = comic.title.substr(n+1);

	parser.GetValue("text", comic.text);
	
	for(bool i=parser.Foreach("images/image"); i; i=parser.Next())
	{
		std::string uri;
		parser.GetValue(".", uri);

		if(!uri.empty())
			comic.images.push_back(uri);
	}
	
	for(bool i=parser.Foreach("texts/text"); i; i=parser.Next())
	{
		std::string v;
		parser.GetValue(".", v);

		if(!v.empty())
			comic.text += v;
	}

	if(!comic.images.empty())
	{
		Comics *comics = (Comics*)param;
		comics->push_back(comic);
	}
	return 0;
}

static int list_parser(void* param, const char* xml)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return ERROR_PARAM;

	std::vector<std::string> * uris = (std::vector<std::string>*)param;
	for(bool i=parser.Foreach("uri"); i; i=parser.Next())
	{
		std::string uri;
		parser.GetValue(".", uri);

		if(!uri.empty())
			uris->push_back(std::string("http://www.yyxj8.com")+uri);
	}

	return 0;
}

int CYaoYao::List()
{
	char name[64] = {0};
	char name2[64] = {0};
	std::string indexFile, imageFile;
	sprintf(name, "index-%s", GetName());
	sprintf(name2, "image-%s", GetName());
	if(!g_config.GetConfig(name, indexFile) || !g_config.GetConfig(name2, imageFile)) // xml file
	{
		printf("joke_get: can't find %s xml file.\n", GetName());
		return ERROR_NOTFOUND;
	}

	char uri[256] = {0};
	for(int page=1; page >0; page++)
	{
		// latest update
		snprintf(uri, sizeof(uri)-1, "http://www.yyxj8.com/yj/list_3_%d.html", page);

		std::vector<std::string> uris;
		int r = web_translate(uri, NULL, indexFile.c_str(), list_parser, &uris);
		printf("CYaoYao::List[%d] = %d.\n", page, r);
		if(0 != r)
			return r;

		Comics comics;
		for(size_t i = 0; i < uris.size(); ++i)
		{
			r = web_translate(uris[i].c_str(), NULL, imageFile.c_str(), image_parser, &comics);
			printf("CYaoYao::List[%d] get image[%s]= %d.\n", page, uris[i].c_str(), r);
		}

		// save
		r = jokedb_insert_comics(GetName(), comics);
		if(r < 0)
			printf("CYaoYao::List[%d] jokedb_insert=%d.\n", page, r);

		system_sleep(5000);
	}
}

int CYaoYao::Hot()
{
	return 0;
}

int CYaoYao::Check()
{
	return 0;
}

int CYaoYao::GetComment(Comments& comments, unsigned int id)
{
	return 0;
}
