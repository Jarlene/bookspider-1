#ifndef _domdoc_h_
#define _domdoc_h_

void* domdoc_create(const char* p);
void domdoc_destroy(void* dom);
const char* domdoc_getencoding(void* dom);
const char* domdoc_geterror(void* dom);

#endif /* !_domdoc_h_ */
