#ifndef _http_h_
#define _http_h_

#include "libhttp.h"

#ifdef  __cplusplus
extern "C" {
#endif

LIBHTTP_API int http_request(const char* uri, const char* req, void** reply, int *len);

#ifdef  __cplusplus
}
#endif

#endif /* !_http_h_ */
