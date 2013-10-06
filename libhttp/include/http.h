#ifndef _http_h_
#define _http_h_

#include "dllexport.h"

#if defined(LIBHTTP_EXPORTS)
	#define LIBHTTP_API DLL_EXPORT_API
#else
	#define LIBHTTP_API DLL_IMPORT_API
#endif

#ifdef  __cplusplus
extern "C" {
#endif

LIBHTTP_API int http_request(const char* uri, const char* req, void** reply);

#ifdef  __cplusplus
}
#endif

#endif /* !_http_h_ */
