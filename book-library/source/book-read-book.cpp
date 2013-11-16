#include "book-site.h"
#include "utf8codec.h"
#include "XMLParser.h"
#include "web-translate.h"
#include "error.h"
#include "config.h"

static int OnReadBook(void* param, const char* xml)
{
	book_info *p = (book_info*)param;

	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	const char* encoding = parser.GetEncoding();

	std::string name, uri, author, category, click, wordcount, datetime;
	parser.GetValue("name", name);
	parser.GetValue("uri", uri);
	parser.GetValue("author", author);
	parser.GetValue("category", category);
	parser.GetValue("click", click);
	parser.GetValue("wordcount", wordcount);
	parser.GetValue("datetime", datetime);

	if(name.empty() || uri.empty() || author.empty())
		return ERROR_PARAM;

	return 0;
}

int read_book(IBookSite* site, const char* uri, const char* req, book_info& book)
{
	char name[64] = {0};
	sprintf(name, "book/web-%s", site->GetName());

	std::string xmlfile;
	if(!g_config.GetConfig(name, xmlfile)) // xml file
	{
		printf("read_book: can't find %s xml file.\n", site->GetName());
		return ERROR_NOTFOUND;
	}

	return web_translate(uri, req, xmlfile.c_str(), OnReadBook, &book);
}
