#include "../algorithm/algorithm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "unicode.h"
#include <string>
#include <vector>
#include "cppstringext.h"
#include "mmptr.h"

static int CheckSpecialCharacter(const char* s)
{
	static char buffer[64] = {0};
	wchar_t* chinese = L" <>():-¡¶¡·£¨£©£º";

	if(0 == buffer[0])
		unicode_to_utf8(chinese, 0,  buffer, sizeof(buffer));

	const char* p = strchr(buffer, *s);
	if(!p)
		return 0;

	int i = 1;
	for(; s[i] == p[i]; i++);
	return i;
}

static void ChapterPreprocess(const char* s, std::string& chapter)
{
	while(s && *s)
	{
		int n = CheckSpecialCharacter(s);
		if(n > 0)
		{
			s += n;
			continue;
		}

		chapter += *s++;
	}
}

static void chapter_split(const char* s, std::vector<std::wstring>& parts)
{
	const wchar_t* special1 = L" <(£¨¡¢¡¡";
	const wchar_t* special2 = L"?\'\",.:>)£©¡¶¡·£º£¬¡£¡¯¡°?";

	mmptr ptr(sizeof(wchar_t)*(strlen(s)+1));
	unicode_from_utf8(s, 0, (wchar_t*)ptr.get(), ptr.capacity());

	std::wstring part;
	for(const wchar_t* ws=(wchar_t*)ptr.get(); *ws; ++ws)
	{
		if(wcschr(special1, *ws))
		{
			if(!part.empty())
			{
				parts.push_back(part);
				part.clear();
			}
		}
		else if(wcschr(special2, *ws))
		{
			// filter
		}
		else
		{
			part += *ws;
		}
	}

	if(!part.empty())
		parts.push_back(part);
}

bool chapter_find(const char* s1, const char* s2)
{
	std::vector<std::wstring> chapters1, chapters2;
	chapter_split(s1, chapters1);
	chapter_split(s2, chapters2);
	if(chapters1.empty() || chapters2.empty())
		return false;

	size_t i1 = chapters1.size()>chapters2.size()?chapters1.size()-chapters2.size():0;
	size_t i2 = chapters1.size()>chapters2.size()?0:chapters2.size()-chapters1.size();
	assert(i1+1<=chapters1.size() && i2+1<=chapters2.size());
	for(; i1<chapters1.size(); ++i1, ++i2)
	{
		assert(i2<chapters2.size());
		if(chapters1[i1] != chapters2[i2])
			return false;
	}
	return true;

	//// filter special character( <>())
	//std::string chapter1, chapter2;
	//ChapterPreprocess(s1, chapter1);
	//ChapterPreprocess(s2, chapter2);

	//// longest common substring
	//char buffer[128] = {0};
	//strsubstring(chapter1.c_str(), chapter2.c_str(), buffer, sizeof(buffer));
}
