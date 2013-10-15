#ifndef _WebSession_h_
#define _WebSession_h_

#include "sys/sock.h"
#include "jsonhelper.h"
#include <string>
#include <map>

class WebSession
{
public:
	WebSession(socket_t sock, const char* ip, int port);
	~WebSession();

public:
	static void Run(void *param);

private:
	void OnApi();
	void Run();

	int Recv();
	int Send(int code, const char* contentType, const void* data, int len);

	int OnBookLibrary(jsonobject& reply);
	int OnBookTop(jsonobject& reply);
	int OnBookList(jsonobject& reply);
	int OnBookUpdate(jsonobject& reply);
	int OnGetStatus(jsonobject& reply);

private:
	std::string m_ip;
	socket_t m_sock;
	int m_port;

	std::string m_path;
	std::map<std::string, std::string> m_params;

	void* m_http;
	void* m_content;
	int m_contentLength;
};

#endif /* !_WebSession_h_ */
