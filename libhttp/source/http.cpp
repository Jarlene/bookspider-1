#include "http.h"
#include "network-http.h"

int http_request(const char* uri, const char* req, void** reply)
{
	assert(uri && reply);

	mmptr ptr;
	int r = network_http(uri, req, ptr);
	if(r < 0)
		return r;

	r = (int)ptr.size();
	*reply = ptr.detach();
	return r;
}
