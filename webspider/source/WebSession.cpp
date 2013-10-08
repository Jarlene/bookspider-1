#include "WebSession.h"
#include "cstringext.h"
#include "sys/path.h"
#include "http-server.h"
#include "error.h"
#include "dlog.h"
#include "config.h"
#include "WebProcess.h"
#include "jsonhelper.h"
#include <time.h>

#define SERVER_TIMEOUT	(5*60*1000)

WebSession::WebSession(socket_t sock, const char* ip, int port)
{
	m_ip = ip;
	m_sock = sock;
	m_port = port;

	m_http = http_server_create(sock);
	dlog_log("new client[%s.%d] connected.\n", ip, port);
}

WebSession::~WebSession()
{
	http_server_destroy(&m_http);
	dlog_log("client[%s.%d] disconnected.\n", m_ip.c_str(), m_port);
}

void WebSession::Run(void* param)
{
	WebSession* session = (WebSession*)param;
	session->Run();
}

void WebSession::Run()
{
	int r = 0;
	assert(socket_invalid != m_sock);
	while(1)
	{
		r = Recv();
		if(r < 0)
			break; // exit

		const char* urlpath = http_server_get_path(m_http);
		if( 0 == strncmp(urlpath, "/api/", 5) )
		{
			OnApi(urlpath+5);
		}
	}

	dlog_log("\n[%d] client %s.%d disconnect[%d].\n", (int)time(NULL), m_ip.c_str(), m_port, r);
	delete this;
}

void WebSession::OnApi(const char* api)
{
	char uri[128] = {0};
	char path[128] = {0};
	std::string app, arg1, arg2;

	snprintf(uri, sizeof(uri)-1, "api/api-%s/app", api);
	g_config.GetConfig(uri, app);

	snprintf(uri, sizeof(uri)-1, "api/api-%s/arg1", api);
	g_config.GetConfig(uri, arg1);

	snprintf(uri, sizeof(uri)-1, "api/api-%s/arg2", api);
	g_config.GetConfig(uri, arg2);

	process_selfname(uri, sizeof(uri));
	path_dirname(uri, path);
	app.insert(0, "/");
	app.insert(0, path);

	jsonobject json;
	if(app.empty() || arg1.empty())
	{
		json.add("code", -1).add("msg", "command not found");
	}
	else
	{
		process_t pid = 0;
		int r = web_process_create(app.c_str(), arg1.c_str(), arg2.c_str(), &pid);
		if(r < 0)
		{
			json.add("code", r).add("msg", "start process error.");
		}
		else
		{
			json.add("code", 0).add("msg", "ok");
		}
	}

	std::string reply = json.json();
	Send(200, "application/json", reply.c_str(), reply.length());
}

int WebSession::Recv()
{
	m_content = NULL;
	m_contentLength = 0;

	int r = socket_select_read(m_sock, SERVER_TIMEOUT);
	if(r <= 0)
		return 0==r ? ERROR_RECV_TIMEOUT : r;

	r = http_server_recv(m_http);
	if(r < 0)
		return r;

	http_server_get_content(m_http, &m_content, &m_contentLength);
	if(m_contentLength > 0 && m_contentLength < 2*1024)
	{
		printf("%s\n", (const char*)m_content);
	}
	return 0;
}

int WebSession::Send(int code, const char* contentType, const void* data, int len)
{
	http_server_set_header(m_http, "Server", "MD WebServer 0.1");
	http_server_set_header(m_http, "Connection", "keep-alive");
	http_server_set_header(m_http, "Keep-Alive", "timeout=5,max=100");
	http_server_set_header(m_http, "Content-Type", contentType);
	//http_server_set_header(m_http, "Content-Type", "text/html; charset=utf-8");
	http_server_set_header_int(m_http, "Content-Length", len);

	int r = http_server_send(m_http, code, (void*)data, len);

	if(len > 0 && len < 2*1024)
	{
		dlog_log("%s\n", (const char*)data);
	}
	return r;
}
