#include "http.h"
#include "network-http.h"

int http_request(const char* uri, const char* req, void** reply, int *len)
{
	assert(uri && reply);

	mmptr ptr;
	int r = network_http(uri, req, ptr);
	if(ptr.size() > 0)
	{
		*len = ptr.size();
		*reply = ptr.detach();
	}

	return r;
}
