#include "joke-db.h"
#include "tcpserver.h"
#include "sys-thread-pool.h"
#include "sys-task-queue.h"
#include "web-session.h"
#include "joke-comment-db.h"
#include "http-proxy.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef OS_LINUX
#include <signal.h>
#endif

int config_proxy_load();

void OnTcpConnected(void* param, socket_t sock, const char* ip, int port)
{
	WebSession* session = new WebSession(sock, ip, port);
	if(0 != sys_thread_pool_push(WebSession::Run, session))
	{
		printf("thread pool push error[%s.%d].\n", ip, port);
		delete session;
	}
}

void OnTcpError(void* param, int errcode)
{
	printf("OnTcpError: %d\n", errcode);
}

sys_task_queue_t g_taskQ;
int main(int argc, char* argv[])
{
#if defined(OS_LINUX)
	/* ignore pipe signal */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, 0);
	sigaction(SIGPIPE, &sa, 0);
#endif

	tcpserver_t tcpserver;
	tcpserver_handler_t tcphandler;
	tcphandler.onerror = OnTcpError;
	tcphandler.onconnected = OnTcpConnected;

	// use proxy
	config_proxy_load();
	http_proxy_add_pattern("*.budejie.com");
	http_proxy_add_pattern("*.qiushibaike.com");
	http_proxy_add_pattern("*.baozoumanhua.com");
	http_proxy_add_pattern("*.yyxj8.com");

	socket_init();
	jokedb_init();
	jokecomment_init();
	g_taskQ = sys_task_queue_create(20);
	tcpserver = tcpserver_start(NULL, 2001, &tcphandler, NULL);

	while('q' != getchar())
	{
	}

	tcpserver_stop(tcpserver);
	sys_task_queue_destroy(g_taskQ);
	jokecomment_save();
	jokedb_clean();
	socket_cleanup();
	return 0;
}
