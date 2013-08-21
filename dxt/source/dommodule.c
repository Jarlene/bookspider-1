#include "domutil.h"
#include "dommodule.h"
#include <assert.h>

#define ARRAYLEN(arr) (sizeof(arr)/sizeof(arr[0]))

static dommodule_t* sg_modules[32];

int dommodule_register(dommodule_t* module)
{
	int i;

	i = ((int)(module)) % ARRAYLEN(sg_modules);
	if(0 == sg_modules[i])
	{
		sg_modules[i] = module;
		return 0;
	}

	for(i=0; i<ARRAYLEN(sg_modules); i++)
	{
		if(0 == sg_modules[i])
		{
			sg_modules[i] = module;
			return 0;
		}
	}

	assert(0);
	return -1;
}

int domcdatareg();
int domcommentreg();
int dommetereg();
int domscriptreg();
int domphpscriptreg();
int xmldeclarationreg();
static void dommodule_autoreg()
{
	static int s_flags = 0;
	if(s_flags)
		return;

	domcdatareg();
	domcommentreg();
	dommetereg();
	domscriptreg();
	domphpscriptreg();
	xmldeclarationreg();
	s_flags = 1;
}

dommodule_t* dommodule_identify(const char* p)
{
	int i;
	dommodule_t* module;

	dommodule_autoreg();
	p = domutil_skip(p);
	if('<' != *p)
		return NULL;

	for(i=0; i<ARRAYLEN(sg_modules); i++)
	{
		module = sg_modules[i];
		if(0 == module)
			continue;

		if(module->identify(p))
			return module;
	}

	return NULL;
}
