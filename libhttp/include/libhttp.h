#ifndef _libhttp_h_
#define _libhttp_h_

#include "dllexport.h"

#if defined(LIBHTTP_EXPORTS)
	#define LIBHTTP_API DLL_EXPORT_API
#else
	#define LIBHTTP_API DLL_IMPORT_API
#endif

#endif /* !_libhttp_h_ */
