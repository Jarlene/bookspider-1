#include "bookspidertools.h"
#include "XMLParser.h"
#include "urlcodec.h"
#include "utf8.h"

int parse_search_result(const char* xml, parse_search_result_fcb fcb, ...)
{
	XMLParser parser(xml);
	if(!parser.Valid())
		return -1;

	char bookname[100];
	char bookauthor[100];
	char bookuri[128];
	char bookchapter[100];
	for(bool i=parser.Foreach("books/book"); i; i=parser.Next())
	{
		std::string name, author, uri, chapter;
		if(!parser.GetValue("name", name) 
			|| !parser.GetValue("author", author)
			|| !parser.GetValue("uri", uri))
			continue;

		parser.GetValue("chapter", chapter);

		// to utf-8
		const char* encoding = parser.GetEncoding();
		to_utf8(name.c_str(), encoding, bookname, sizeof(bookname));
		to_utf8(author.c_str(), encoding, bookauthor, sizeof(bookauthor));
		to_utf8(uri.c_str(), encoding, bookuri, sizeof(bookuri));
		to_utf8(chapter.c_str(), encoding, bookchapter, sizeof(bookchapter));
		
		// data filter
		va_list args;
		va_start(args, fcb);
		int r = fcb(bookname, bookauthor, bookuri, bookchapter, args);
		va_end(args);

		if(r)
			return r; // find
	}
	return -1; // not found
}

int url_encode_utf8(const char* src, char* dst, unsigned int len)
{
	char buffer[256] = {0};
	
	int r = utf8_to_gb18030(src, buffer, sizeof(buffer));
	r = url_encode(buffer, -1, dst, len);
	return r;
}
