#include "domutil.h"
#include <stdlib.h>
#include <assert.h>

char* domutil_strdup(const char* p, unsigned int n)
{
	char* s;
	n = n?n:strlen(p);

	s = (char*)malloc(n+1);
	if(!s)
		return NULL;

	strncpy(s, p, n);
	s[n] = 0;
	return s;
}

const char* domutil_skip(const char* p)
{
	assert(p);
	while(*p && strchr(" \t\r\n", *p))
		++p;
	return p;
}

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
// Including the basic of this table, which determines the #bytes in the
// sequence from the lead byte. 1 placed for invalid sequences --
// although the result will be junk, pass it through as much as possible.
// Beware of the non-characters in UTF-8:	
//				ef bb bf (Microsoft "lead bytes")
//				ef bf be
//				ef bf bf 
const char* domutil_skipbom(const char* p)
{
	assert(p);
	if((*(p+0)==(char)0xef && *(p+1)==(char)0xbb && *(p+2)==(char)0xbf)
		|| (*(p+0)==(char)0xef && *(p+1)==(char)0xbf && *(p+2)==(char)0xbe)
		|| (*(p+0)==(char)0xef && *(p+1)==(char)0xbf && *(p+2)==(char)0xbf))
		return p+3;
	return p;
}

#define is_lower(c)		(c>='a' && c<='z')
#define is_upper(c)		(c>='A' && c<='Z')
#define is_alpha(c)		(is_lower(c) || is_upper(c))
#define is_number(c)	(c>='0' && c<='9')
#define is_alnum(c)		(is_alpha(c) || is_number(c))

// p = tokenname("abc='def'") 
// => p = "='def'"
const char* domutil_tokenname(const char* p)
{
	// <?xml:ns ?>
	// <?php?>
	// <!-- -->
	// width=80%
	//while(*p && (is_alnum(*p) || strchr("_-!?:%", *p)))
	while(*p && !strchr(" =</>\t\r\n\'\"", *p))
		++p;
	return p;
}

// tokenvalue("<body onload='OnLoad()'>") 
// => value ='OnLoad'
// => p = ">"
const char* domutil_tokenvalue(const char* p)
{
	switch(*p)
	{
	case '\'':
		p = strchr(p+1, '\'');
		return p ? p+1 : NULL;

	case '\"':
		p = strchr(p+1, '\"');
		return p ? p+1 : NULL;

	default:
		while(*p && !strchr(" \t\r\n>", *p))
		{
			if(('?'==*p || '/'==*p) && '>'==p[1])
				break;
			++p;
		}
		return p;
	}
}

int domutil_cmptagname(const char* p, const char* tagname)
{
	size_t n = strlen(tagname);

	p = domutil_skip(p);
	if('<' != *p)
		return 0;

	p = domutil_skip(p+1);
	if(0==strnicmp(p, tagname, n) && strchr(" \t\r\n?/>", p[n]))
		return 1;
	return 0;
}
