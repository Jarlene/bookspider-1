#include "tcpserver.h"
#include "thread-pool.h"
#include "WebSession.h"
#include <stdio.h>
#include <stdlib.h>

thread_pool_t g_thdpool;

void OnTcpConnected(void* param, socket_t sock, const char* ip, int port)
{
	WebSession* session = new WebSession(sock, ip, port);
	if(0 != thread_pool_push(g_thdpool, WebSession::Run, session))
	{
		printf("thread pool push error[%s.%d].\n", ip, port);
		delete session;
	}
}

void OnTcpError(void* param, int errcode)
{
	printf("OnTcpError: %d\n", errcode);
}

int main(int argc, char* argv[])
{
	tcpserver_t tcpserver;
	tcpserver_handler_t tcphandler;
	tcphandler.onerror = OnTcpError;
	tcphandler.onconnected = OnTcpConnected;

	socket_init();
	WebSpider(spider);
	g_thdpool = thread_pool_create(2, 1, 64);
	tcpserver = tcpserver_start(NULL, 10000, &tcphandler, NULL);

	while('q' != getch())
	{
	}

	tcpserver_stop(tcpserver);
	thread_pool_destroy(g_thdpool);
	socket_cleanup();
	return 0;
}
