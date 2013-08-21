#ifndef _domnode_h_
#define _domnode_h_

#include "domparser.h"

domnode_t* domnode_create();

int domnode_destroy(domnode_t* node);

const char* domnode_parse(domdoc_t* doc, domnode_t* node, const char* p);

void domnode_append(domnode_t* parent, domnode_t* node);

int domnode_setname(domnode_t* node, const char* name, int nameLen);

int domnode_setpadding(domnode_t* node, const char* padding, int paddingLen);

void domnode_attr_append(domnode_t* node, domattr_t* attr);

void domnode_attr_delete(domnode_t* node, domattr_t* attr);

const char* domnode_attr_parse(domnode_t* node, const char* p);

const char* domnode_attr_find(const domnode_t* node, const char* name);

#endif /* !_domnode_h_ */
