#ifndef _DXT_H_
#define _DXT_H_

#include <stdarg.h>
#include "dllexport.h"

#ifndef IN
#define IN 
#endif

#ifndef OUT
#define OUT
#endif

#ifndef CONST
#ifdef  __cplusplus
	#define CONST const
#else
	#define CONST
#endif
#endif

#ifdef DXT_EXPORTS
	#define DXT_API DLL_EXPORT_API
#else
	#define DXT_API DLL_IMPORT_API
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/// ת��
/// @param[in] inStream ����
/// @param[in] xml XML����
/// @param[out] outStream(���ͷ�)
/// @return ������
DXT_API int DXTransform(OUT char** outStream, IN CONST char* inStream, IN int num, ...);
DXT_API int DXTransformV(OUT char** outStream, IN CONST char* inStream, IN int num, va_list val);

/// ת��
/// @param[in] filename ����
/// @param[in] xml XML����
/// @param[out] outStream(���ͷ�)
/// @return ������
DXT_API int DXTransformFile(OUT char** outStream, IN CONST char* filename, IN int num, ...);
DXT_API int DXTransformFileV(OUT char** outStream, IN CONST char* filename, IN int num, va_list val);

DXT_API int DxTransformHtml(OUT char** outStream, IN CONST char* src, IN CONST char* dstFile);
DXT_API int DxTransformHtml2(OUT char** outStream, IN CONST char* srcFile, IN CONST char* dstFile);

#ifdef  __cplusplus
} // extern "C"
#endif

#endif /* !_DXT_H_ */
