#ifndef _domattr_h_
#define _domattr_h_

#include "domparser.h"

domattr_t* domattr_create();
int domattr_destroy(domattr_t* attr);

const char* domattr_parse(const char* p, domattr_t* attr);

int domattr_setname(domattr_t* attr, const char* name, int nameLen);

int domattr_setvalue(domattr_t* attr, const char* value, int valueLen);

#endif /* !_domattr_h_ */
