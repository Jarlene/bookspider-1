#ifndef _bookspider_h_
#define _bookspider_h_

#include <string>

#if defined(_WIN32) || defined(__CYGWIN__)
	#define BOOKSPIDER_API_EXPORT __declspec(dllexport)
	#define BOOKSPIDER_API_IMPORT __declspec(dllimport)
#else
	#if __GNUC__ >= 4
		#define BOOKSPIDER_API_EXPORT __attribute__((visibility ("default")))
		#define BOOKSPIDER_API_IMPORT __attribute__((visibility ("default")))
	#else
		#define BOOKSPIDER_API_EXPORT
		#define BOOKSPIDER_API_IMPORT
	#endif
#endif

#ifdef BOOKSPIDER_EXPORTS
	#define BOOKSPIDER_API BOOKSPIDER_API_EXPORT
#else
	#define BOOKSPIDER_API BOOKSPIDER_API_IMPORT
#endif

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct  
{
	const char* name;
	const char* index;	// index pattern xml file
	int interval;		// search interval

	int (*search)(const char* name, const char* author, char bookUri[128], char indexUri[128]);
} book_spider_t;

BOOKSPIDER_API int book_spider_register(const book_spider_t* spider);

BOOKSPIDER_API int book_spider_count();

BOOKSPIDER_API const book_spider_t* book_spider_get(int index);


#ifdef  __cplusplus
}  // extern "C"
#endif

#endif /* !_bookspider_h_ */
