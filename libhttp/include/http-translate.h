#ifndef _http_translate_
#define _http_translate_

#include "dllexport.h"

#if defined(LIBHTTP_EXPORTS)
	#define LIBHTTPTRANSLATE_API DLL_EXPORT_API
#else
	#define LIBHTTPTRANSLATE_API DLL_IMPORT_API
#endif

#ifdef  __cplusplus
extern "C" {
#endif

typedef int (*OnTranslated)(void* param, const char* xml);

/// HTTP Translate
/// @param[in] uri uniform request index
/// @param[in] req HTTP request
/// @param[in] xml translate xml template
/// @param[in] callback call on translate success
/// @param[in] param callback param
/// @return 0-ok, <0-error
LIBHTTPTRANSLATE_API int http_translate(const char* uri, 
										const char* req, 
										const char* xml, 
										OnTranslated callback, 
										void* param);

#ifdef  __cplusplus
}
#endif

#endif /* !_http_translate_ */
