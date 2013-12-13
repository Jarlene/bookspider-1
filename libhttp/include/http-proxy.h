#ifndef _http_proxy_h_
#define _http_proxy_h_

#include "libhttp.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef char host_t[64];
typedef void (*http_proxy_proc)(void* param, const char* value);

/// add a proxy
/// @param[in] proxy ip address, e.g. 10.1.1.10:2000
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_add(const host_t proxy);

/// delete a proxy
/// @param[in] proxy ip address, e.g. 10.1.1.10:2000
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_delete(const host_t proxy);

/// enum proxies
/// @param[in] proc callback function, call once on each proxy
/// @param[in] param callback parameter
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_list(http_proxy_proc proc, void* param);

/// get proxy count
/// @return proxy count
int LIBHTTP_API http_proxy_count();

/// add a pattern
/// @param[in] pattern host pattern, like: *.google.com
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_add_pattern(const char* pattern);

/// add a pattern
/// @param[in] pattern host pattern
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_delete_pattern(const char* pattern);

/// enum patterns
/// @param[in] proc callback function, call once on each proxy
/// @param[in] param callback parameter
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_list_pattern(http_proxy_proc proc, void* param);

/// add a white pattern(disable proxy, don't use proxy)
/// @param[in] pattern host pattern, like: *.baidu.com
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_add_allow_pattern(const char* pattern);

/// delete a white pattern
/// @param[in] pattern host pattern
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_delete_allow_pattern(const char* pattern);

/// enum white patterns
/// @param[in] proc callback function, call once on each proxy
/// @param[in] param callback parameter
/// @return 0-ok, <0-error
int LIBHTTP_API http_proxy_list_allow_pattern(http_proxy_proc proc, void* param);

#ifdef  __cplusplus
}
#endif

#endif /* !_http_proxy_h_ */
