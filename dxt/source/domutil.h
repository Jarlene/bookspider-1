#ifndef _domutil_h_
#define _domutil_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOMERR_ERROR  -1
#define DOMERR_MEMORY -11

#define FREE(p)  do{if(p) free(p);}while(0)

#if defined(_WIN32)
	#if !defined(__cplusplus)
		#define inline static
	#endif
	
	#define stricmp		_stricmp
	#define strnicmp	_strnicmp
#else
	#if !defined(__cplusplus)
		#define inline static __attribute__((unused))
	#endif

	#ifndef stricmp
		#define stricmp strcasecmp
	#endif

	#ifndef strnicmp
		#define strnicmp strncasecmp
	#endif

	#ifndef _snprintf
		#define _snprintf snprintf
	#endif
#endif

char* domutil_strdup(const char* p, unsigned int n);

const char* domutil_skip(const char* p);
const char* domutil_skipbom(const char* p);

const char* domutil_tokenname(const char* p);

const char* domutil_tokenvalue(const char* p);


int domutil_cmptagname(const char* p, const char* tagname);

#endif /* !_domutil_h_ */
