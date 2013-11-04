#ifndef _WebSession_h_
#define _WebSession_h_

#include "sys/sock.h"
#include "jsonhelper.h"
#include "uri-params.h"
#include "error.h"
#include <string>

class WebSession
{
public:
	WebSession(socket_t sock, const char* ip, int port);
	~WebSession();

public:
	static void Run(void *param);

public:
	int Reply(int code, const char* msg);
	int Reply(const std::string& reply);

private:
	void OnApi();
	void Run();

	int Recv();
	int Send(int code, const char* contentType, const void* data, int len);

	int OnProxy();
	int OnAddProxy();
	int OnDelProxy();
	int OnListProxy();
	int OnComment();
	int OnCleanup();

private:
	std::string m_ip;
	socket_t m_sock;
	int m_port;

	std::string m_path;
	URIParams m_params;

	void* m_http;
	void* m_content;
	int m_contentLength;
};

#endif /* !_WebSession_h_ */
