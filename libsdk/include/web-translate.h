#ifndef _web_translate_
#define _web_translate_

typedef int (*OnTranslated)(void* param, const char* xml);

/// HTTP Translate
/// @param[in] uri uniform request index
/// @param[in] req HTTP request
/// @param[in] xml translate xml template
/// @param[in] callback call on translate success
/// @param[in] param callback param
/// @return 0-ok, <0-error
int web_translate(const char* uri, 
				const char* req, 
				const char* xml, 
				OnTranslated callback, 
				void* param);


#endif /* !_web_translate_ */
