#ifndef _WebSession_h_
#define _WebSession_h_

#include "sys/sock.h"
#include "aio-socket.h"
#include "http-parser.h"
#include "libct/object.h"
#include "libct/auto_obj.h"
#include "jsonhelper.h"
#include "uri-params.h"
#include "joke-list.h"
#include "sys-timer.h"
#include "error.h"
#include <string>

class WebSession : public libct::object
{
public:
	WebSession(socket_t sock, const char* ip, int port);
	~WebSession();

	void Run();

	static void OnJokeTimer(sys_timer_t id, void* param);
	static void On18PlusTimer(sys_timer_t id, void* param);

public:
	int ReplyArrary(const char* name, const std::string& value);
	int Reply(int code, const char* msg);
	int Reply(const std::string& reply);
	int ReplyRedirectTo(const char* uri);

private:
	void OnApi();

	int OnProxy();
	int OnAddProxy();
	int OnDelProxy();
	int OnListProxy();
	int OnComment();
	int OnCleanup();

	int On18Plus();
	int OnHotText();
	int OnHotImage();
	int OnLateText();
	int OnLateImage();

private:
	static void OnRecv(void* param, int code, int bytes);
	static void OnSend(void* param, int code, int bytes);
	int Send(int code, const char* contentType, const void* data, int len);

	int OnJoke(struct rwlist* list, const char* redirect);


private:
	char m_buffer[2*1024];
	char m_buffer2[2*1024];
	aio_socket_t m_sock;
	socket_bufvec_t m_vec[2];
	std::string m_ip;	
	int m_port;

	std::string m_path;
	URIParams m_params;

	void* m_http;
	void* m_content;
	int m_contentLength;
};

#endif /* !_WebSession_h_ */
