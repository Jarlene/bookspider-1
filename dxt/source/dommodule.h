#ifndef _dommodule_h_
#define _dommodule_h_

#include "domparser.h"

typedef struct
{
	int (*identify)(const char* p);
	const char* (*parse)(domdoc_t* doc, domnode_t* node, const char* p);
} dommodule_t;

int dommodule_register(dommodule_t* module);

dommodule_t* dommodule_identify(const char* p);

#endif /* !_dommodule_h_ */
