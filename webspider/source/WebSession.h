#ifndef _WebSession_h_
#define _WebSession_h_

#include "sys/sock.h"
#include <string>

class WebSession
{
public:
	WebSession(socket_t sock, const char* ip, int port);
	~WebSession();

public:
	static void Run(void *param);

private:
	void OnApi(const char* api);
	void Run();

	int Recv();
	int Send(int code, const char* contentType, const void* data, int len);

private:
	std::string m_ip;
	socket_t m_sock;
	int m_port;

	void* m_http;
	void* m_content;
	int m_contentLength;
};

#endif /* !_WebSession_h_ */
