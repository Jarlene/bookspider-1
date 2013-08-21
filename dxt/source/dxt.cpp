// dxt.cpp : Defines the exported functions for the DLL application.
//

#include "dxt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "StdCFile.h"
#include "XmlSource.h"
#include "dom.h"
extern "C"
{
#include "domdoc.h"
}

#ifndef MemAlloc
	#define MemAlloc malloc
#endif

#ifndef MemFree
	#define MemFree free
#endif

char* strcopyA(IN CONST char* source)
{
	size_t n = strlen(source);
	char* p = (char*)MemAlloc((n+1)*sizeof(char));
	if(NULL == p)
		return NULL;
	strcpy(p, source);
	return p;
}

wchar_t* strcopyW(IN CONST wchar_t* source)
{
	size_t n = wcslen(source);
	wchar_t* p = (wchar_t*)MemAlloc((n+1)*sizeof(wchar_t));
	if(NULL == p)
		return NULL;
	wcscpy(p, source);
	return p;
}

class AutoMemFree
{
	AutoMemFree(const AutoMemFree&){}
	AutoMemFree& operator =(const AutoMemFree&){ return *this; }

public:
	AutoMemFree(size_t n)
	{	m_p = MemAlloc(n); }

	AutoMemFree(void* p)
	{	m_p = p; }

	~AutoMemFree()
	{	if(m_p)	MemFree(m_p);	}

	operator void*&()
	{	return m_p;	}

	operator void*()
	{	return m_p;	}

	operator const void*() const
	{	return m_p;	}

public:
	bool Valid() const
	{	return NULL != m_p;	}

	void* Attach(void* p)
	{
		if(m_p)
			MemFree(m_p);
		m_p = p;
		return p;
	}

	void* Detach()
	{	return m_p;	}

	void* Get()
	{	return m_p;	}

	const void* Get() const
	{	return m_p;	}

private:
	void* m_p;
};

int domdoc_translate(void* dom, ISource* xml, std::string& output);

static void InitXmlSource(XmlSource& source, int num, va_list val)
{
	for(int i=0; i<num; i++)
	{
		const char* xml = va_arg(val, const char*);
		if(NULL == xml)
			break;

		if(!source.Add(xml))
			printf("Translate parse xml error.");
	}
}

static int DomTranslate(char** outStream, const char* inStream, ISource* source)
{
	void* doc = domdoc_create(inStream);
	if(!doc || domdoc_geterror(doc))
	{
		const char* p = domdoc_geterror(doc);
		*outStream = strcopyA(p);
		return -1;
	}

	std::string output;
	domdoc_translate(doc, source, output);
	const char* p = domdoc_geterror(doc);
	if(p)
	{
		*outStream = strcopyA(p);
	}
	else
	{
		*outStream = strcopyA(output.c_str());
	}
	domdoc_destroy(doc);
	return p?-1:0;
}

int DXTransform(OUT char** outStream, IN CONST char* inStream, IN int num, ...)
{
	va_list va;
	va_start(va, num);
	int r = DXTransformV(outStream, inStream, num, va);
	va_end(va);
	return r;
}

int DXTransformV(OUT char** outStream, IN CONST char* inStream, IN int num, va_list val)
{
	XmlSource source;
	InitXmlSource(source, num, val);
	return DomTranslate(outStream, inStream, &source);
}

int DXTransformFile(OUT char** outStream, IN CONST char* filename, IN int num, ...)
{
	StdCFile file(filename, "rb");
	if(!file.IsOpened())
		return -file.GetError();

	AutoMemFree p(file.Read());
	if(!p.Valid())
		return -file.GetError();

	va_list va;
	va_start(va, num);
	int r = DXTransformV(outStream, (const char*)p.Get(), num, va);
	va_end(va);
	return r;
}

int DXTransformFileV(OUT char** outStream, IN CONST char* filename, IN int num, va_list val)
{
	StdCFile file(filename, "rb");
	if(!file.IsOpened())
		return -file.GetError();

	AutoMemFree p(file.Read());
	if(!p.Valid())
		return -file.GetError();

	// notice:
	//  must be sure p end with null-character
	//  p[end] = 0
	int r = DXTransformV(outStream, (const char*)p.Get(), num, val);
	return r;
}

int DxTransformHtml(OUT char** outStream, IN CONST char* src, IN CONST char* dstFile)
{
	StdCFile file(dstFile, "rb");
	if(!file.IsOpened())
		return -file.GetError();

	AutoMemFree p(file.Read());
	if(!p.Valid())
		return -file.GetError();

	void* doc = domdoc_create(src);
	if(!doc || domdoc_geterror(doc))
	{
		const char* p = domdoc_geterror(doc);
		*outStream = strcopyA(p);
		return -1;
	}

	dom source(doc);
	int r = DomTranslate(outStream, (const char*)p.Get(), &source);
	domdoc_destroy(doc);
	return r;
}

int DxTransformHtml2(OUT char** outStream, IN CONST char* srcFile, IN CONST char* dstFile)
{
	StdCFile file(srcFile, "rb");
	if(!file.IsOpened())
		return -file.GetError();

	AutoMemFree p(file.Read());
	if(!p.Valid())
		return -file.GetError();

	return DxTransformHtml(outStream, (CONST char*)p.Get(), dstFile);
}
