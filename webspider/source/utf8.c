#include "utf8.h"
#include "cstringext.h"
#include "unicode.h"
#include <wchar.h>
#include <string.h>
#include <assert.h>

int utf8_to_gb18030(const char* src, char* tgt, int tgtBytes)
{
	int n;
	wchar_t g_wbuf[512];
	
	n = strlen(src)>100?100:0;
	n = unicode_from_utf8(src, n, g_wbuf, sizeof(g_wbuf));

	g_wbuf[30] = 0;
	unicode_to_gb18030(g_wbuf, 0, tgt, tgtBytes);
	return 0;
}

int gb18030_to_utf8(const char* src, char* tgt, int tgtBytes)
{
	int n;
	wchar_t g_wbuf[512];

	n = strlen(src)>100?100:0;
	n = unicode_from_gb18030(src, 0, g_wbuf, sizeof(g_wbuf));

	g_wbuf[30] = 0;
	unicode_to_utf8(g_wbuf, 0, tgt, tgtBytes);
	return 0;
}

int to_utf8(const char* text, const char* encoding, char* utf8, int utf8Len)
{
	assert(text);
	if(strieq("utf-8", encoding))
	{
		strncpy(utf8, text, utf8Len-1);
		utf8[utf8Len-1] = 0;
		return 0;
	}
	else if(strieq("gbk", encoding) || strieq("gb2312", encoding) || strieq("gb18030", encoding))
	{
		return gb18030_to_utf8(text, utf8, utf8Len);
	}
	else
	{
		assert(0);
		return -1;
	}
}
