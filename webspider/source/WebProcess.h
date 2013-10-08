#ifndef _WebProcess_h_
#define _WebProcess_h_

#include "sys/process.h"

#ifdef __cplusplus
extern "C"{
#endif

/// create new process
/// @param appname process exec file
/// @param arg 
/// @return 0-ok, other-error
int web_process_create(const char* appname, const char* arg1, const char* arg2, process_t *pid);

/// find process
/// @param appname process exec file
/// @param arg 
/// @return 0-find it, other-don't exist
int web_process_find(const char* appname, const char* arg, process_t *pid);

#ifdef __cplusplus
}
#endif

#endif /* !_WebProcess_h_ */
