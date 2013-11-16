#include "book-site.h"
#include "utf8codec.h"
#include "XMLParser.h"
#include "web-translate.h"
#include "error.h"
#include "config.h"

static int OnReadChapter(void* param, const char* xml)
{
	std::string *p = (std::string*)param;

	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	const char* encoding = parser.GetEncoding();

	for(bool i=parser.Foreach("p"); i; i=parser.Next())
	{
		std::string paragraph;
		parser.GetValue(".", paragraph);
		if(paragraph.empty())
			continue;

		if(!p->empty()) p->append("\r\n");

		p->append(UTF8Encode(paragraph.c_str(), encoding)); // to utf-8
	}

	return 0;
}

int read_chapter(IBookSite* site, const char* uri, const char* req, std::string& chapter)
{
	char name[64] = {0};
	sprintf(name, "chapter/web-%s", site->GetName());

	std::string xmlfile;
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("read_chapter: can't find %s xml file.\n", site->GetName());
		return ERROR_NOTFOUND;
	}

	return web_translate(uri, req, xmlfile.c_str(), OnReadChapter, &chapter);
}
