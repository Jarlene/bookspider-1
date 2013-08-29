#include <stdarg.h>
#include "cstringext.h"
#include "sys/system.h"

enum EFileAttributes
{
	EFA_READONLY	= 0x01,
	EFA_HIDDEN		= 0x02,
	EFA_DIRECTORY	= 0x10,
};

typedef int (*FCBListFile)(const char* filename, int attributes, va_list args);

static int ListFiles(const char* dir, FCBListFile fcb, ...)
{
#if defined(_WIN32) || defined(_WIN64)
	int attributes;
	va_list args;
	WIN32_FIND_DATA data;
	HANDLE h;
	
	h = FindFirstFile(dir, &data);
	if(INVALID_HANDLE_VALUE == h)
		return -(int)GetLastError();

	do 
	{
		attributes = 0;
		if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			attributes |= EFA_DIRECTORY;
		if(data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			attributes |= EFA_HIDDEN;
		if(data.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			attributes |= EFA_READONLY;

		va_start(args, fcb);
		fcb(data.cFileName, attributes, args);
		va_end(args);

	} while (FindNextFile(h, &data));

	FindClose(h);
	return 0;
#else
#endif
}

static int OnListFile(const char* filename, int attributes, va_list args)
{
#if defined(_WIN32) || defined(_WIN64)
	HMODULE h = system_load(filename);
	system_unload(h);
#endif
	return 0;
}
