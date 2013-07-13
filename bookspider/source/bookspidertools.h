#ifndef _bookspidertools_h_
#define _bookspidertools_h_

#include <stdarg.h>

typedef int (*parse_search_result_fcb)(const char* name, const char* author, const char* uri, const char* chapter, va_list args);

int parse_search_result(const char* xml, parse_search_result_fcb fcb, ...);

int url_encode_utf8(const char* src, char* dst, size_t len);

#endif /* !_bookspidertools_h_ */
