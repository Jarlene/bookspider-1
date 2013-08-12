#include "netmanager.h"
#include "XMLParser.h"
#include "utf8.h"
#include <list>
#include <vector>

static int parse_book_chapter(const char* xml, std::list<std::string>& chapters)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	char bookchapter[100];
	for(bool i=parser.Foreach("chapters/chapter"); i; i=parser.Next())
	{
		std::string name;
		if(!parser.GetValue("name", name))
			continue;

		// to utf-8
		memset(bookchapter, 0, sizeof(bookchapter));
		const char* encoding = parser.GetEncoding();
		to_utf8(name.c_str(), encoding, bookchapter, sizeof(bookchapter));

		// data filter
		if(*bookchapter)
			chapters.push_back(bookchapter);
	}

	bool ascending = true;
	if(parser.GetValue("ascending", ascending) && !ascending)
		chapters.reverse(); // descending
	return 0;
}

int ReadBook(const char* uri, const char* xml, std::list<std::string>& chapters)
{
	char* result;
	int r = WebToXml(uri, NULL, xml, &result);
	if(r)
		return r;

	r = parse_book_chapter(result, chapters);
	free(result);
	return r;
}

#include "StdCFile.h"
int ReadChapter(const char* filename, std::list<std::string>& chapters)
{
	StdCFile file(filename, "rb");
	char* xml = (char*)file.Read(0);

	parse_book_chapter(xml, chapters);
	free(xml);
	return 0;
}
