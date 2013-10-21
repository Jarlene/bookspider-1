#ifndef _network_http_h_
#define _network_http_h_

#include "mmptr.h"

int network_http(const char* uri, const char* req, mmptr& reply);

#endif /* !_network_http_h_ */
