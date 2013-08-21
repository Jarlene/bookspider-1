#ifndef _http_h_
#define _http_h_

#include "dllexport.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef NET_EXPORTS
	#define NET_API DLL_EXPORT_API
#else
	#define NET_API DLL_IMPORT_API
#endif

NET_API void* http_open();

NET_API void http_close(void* http);

NET_API int http_get(void* http, const char* uri, void** reply);
NET_API int http_post(void* http, const char* uri, const void* req, unsigned int len, void** reply);

NET_API int http_set_header(void* http, const char* name, const char* value);
NET_API int http_get_header(void* http, const char* name, char* value, unsigned int len);

NET_API void http_set_timeout(void* http, int timeout);

#ifdef  __cplusplus
}
#endif

#endif /* !_http_h_ */
