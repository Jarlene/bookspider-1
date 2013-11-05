#ifndef _http_proxy_h_
#define _http_proxy_h_

#include "libhttp.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef char host_t[64];
typedef void (*http_proxy_proc)(void* param, const char* value);

int LIBHTTP_API http_proxy_add(const host_t proxy);

int LIBHTTP_API http_proxy_delete(const host_t proxy);

int LIBHTTP_API http_proxy_list(http_proxy_proc proc, void* param);

int LIBHTTP_API http_proxy_count();

int LIBHTTP_API http_proxy_add_pattern(const char* pattern);

int LIBHTTP_API http_proxy_delete_pattern(const char* pattern);

int LIBHTTP_API http_proxy_list_pattern(http_proxy_proc proc, void* param);

int LIBHTTP_API http_proxy_add_allow_pattern(const char* pattern);

int LIBHTTP_API http_proxy_delete_allow_pattern(const char* pattern);

int LIBHTTP_API http_proxy_list_allow_pattern(http_proxy_proc proc, void* param);

#ifdef  __cplusplus
}
#endif

#endif /* !_http_proxy_h_ */
