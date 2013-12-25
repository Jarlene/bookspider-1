#include "web-session.h"
#include "cstringext.h"
#include "dlog.h"
#include "url.h"
#include "config.h"
#include <time.h>

#define SERVER_TIMEOUT	(5*60*1000)

WebSession::WebSession(socket_t sock, const char* ip, int port)
{
	m_ip = ip;
	m_port = port;

	m_sock = aio_socket_create(sock, 1);
	m_http = http_parser_create(HTTP_PARSER_SERVER);
	dlog_log("new client[%s.%d] connected.\n", ip, port);
}

WebSession::~WebSession()
{
	aio_socket_destroy(m_sock);
	if(m_http)
		http_parser_destroy(m_http);
	dlog_log("client[%s.%d] disconnected.\n", m_ip.c_str(), m_port);
}

void WebSession::OnRecv(void* param, int code, int bytes)
{
	WebSession *self = (WebSession*)param;
	if(0 == code && bytes > 0)
	{
		int remain = bytes;
		code = http_parser_input(self->m_http, self->m_buffer, &remain);
		if(0 == code)
		{
			self->OnApi();

			http_parser_clear(self->m_http);
		}
		else if(1 == code)
		{
			code = aio_socket_recv(self->m_sock, self->m_buffer, sizeof(self->m_buffer), OnRecv, self);
		}
	}

	if(code < 0 || 0 == bytes)
	{
		self->release();
		dlog_log("\n[%d] WebSession::OnRecv error: %d\n", (int)time(NULL), 0==bytes ? 0 : code);
	}
}

void WebSession::Run()
{
	if(0 != aio_socket_recv(m_sock, m_buffer, sizeof(m_buffer), OnRecv, this))
	{
		release();
	}
}

void WebSession::OnApi()
{
	m_content = (void*)http_get_content(m_http);
	m_contentLength = http_get_content_length(m_http);
	if(m_contentLength > 0 && m_contentLength < 2*1024)
	{
		printf("%s\n", (const char*)m_content);
	}

	void* url = url_parse(http_get_request_uri(m_http));
	m_path.assign(url_getpath(url));
	m_params.Init(url);
	url_free(url);
	printf("[%s] %s\n", m_ip.c_str(), m_path.c_str());

	typedef int (WebSession::*Handler)();
	typedef std::map<std::string, Handler> THandlers;
	static THandlers handlers;
	if(0 == handlers.size())
	{
		handlers.insert(std::make_pair("proxy", &WebSession::OnProxy));
		handlers.insert(std::make_pair("comment", &WebSession::OnComment));
		handlers.insert(std::make_pair("cleanup", &WebSession::OnCleanup));
		handlers.insert(std::make_pair("addproxy", &WebSession::OnAddProxy));
		handlers.insert(std::make_pair("delproxy", &WebSession::OnDelProxy));
		handlers.insert(std::make_pair("listproxy", &WebSession::OnListProxy));
	}

	if(0 == strncmp(m_path.c_str(), "/api/", 5))
	{
		THandlers::iterator it;
		it = handlers.find(m_path.substr(5));
		if(it != handlers.end())
		{
			(this->*(it->second))();
			return;
		}
	}

	Reply(ERROR_NOTFOUND, "command not found");
}

int WebSession::OnCleanup()
{
	return 0;
}

int WebSession::ReplyArrary(const char* name, const std::string& value)
{
	std::string reply = "{ \"code\" : 0, \"msg\" : \"ok\", \"data\" :";
	reply += value;
	reply += "}";
	return Reply(reply);
}

int WebSession::Reply(int code, const char* msg)
{
	jsonobject json;
	json.add("code", code);
	json.add("msg", msg);
	std::string reply = json.json();
	return Reply(reply);
}

int WebSession::Reply(const std::string& reply)
{
	return Send(200, "application/json", reply.c_str(), reply.length());
}

void WebSession::OnSend(void* param, int code, int bytes)
{
	WebSession *self = (WebSession*)param;
	if(0 == code)
		code = aio_socket_recv(self->m_sock, self->m_buffer, sizeof(self->m_buffer), OnRecv, self);

	if(code < 0)
		self->release();
}

int WebSession::Send(int code, const char* contentType, const void* data, int len)
{
	sprintf(m_buffer2, "HTTP/1.1 %d OK\r\n"
					"Server: MD WebServer 0.1\r\n"
					"Connection: keep-alive\r\n"
					"Keep-Alive: timeout=5,max=100\r\n"
					"Content-Type: %s\r\n"
					//"Content-Type: text/html; charset=utf-8"
					"Content-Length: %d\r\n\r\n", 
					code, contentType, len);

	if(len > 0 && len < 2*1024)
	{
		dlog_log("%s\n", (const char*)data);
	}

	socket_bufvec_t vec[2];
	socket_setbufvec(vec, 0, m_buffer2, strlen(m_buffer2));
	socket_setbufvec(vec, 1, (void*)data, len);
	return aio_socket_send_v(m_sock, vec, 2, OnSend, this);
}
