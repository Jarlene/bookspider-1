#include "sys/sock.h"
#include "sys/system.h"
#include "tcpserver.h"
#include "aio-socket.h"
#include "sys-thread-pool.h"
#include "sys-task-queue.h"
#include "web-session.h"
#include "joke-comment-db.h"
#include "http-proxy.h"
#include "config.h"
#include "joke-db.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef OS_LINUX
#include <signal.h>
#endif

static int s_workers = 0;
static aio_socket_t s_webserver;
sys_task_queue_t g_taskQ;

int config_proxy_load();

void OnWorker(void* param)
{
	while(1)
	{
		int r = aio_socket_process();
		if(0 != r)
		{
			printf("aio socket error: %d\n", r);
		}
	}
}

void OnAccept(void*, int code, socket_t socket, const char* ip, int port)
{
	if(0 != code)
	{
		printf("aio socket accept error: %d/%d.\n", code, socket_geterror());
		exit(1);
	}

	printf("aio socket accept %s.%d\n", ip, port);

	// listen
	aio_socket_accept(s_webserver, OnAccept, NULL);

	WebSession* session = new WebSession(socket, ip, port);
	session->Run();
}

static int InitWebServer(const char* ip, int port)
{
	socket_t server = tcpserver_create(ip, port, 256);
	if(0 == server)
	{
		printf("server listen at %s.%d error: %d\n", ip?ip:"127.0.0.1", port, socket_geterror());
		return -1;
	}

	s_webserver = aio_socket_create(server, 1);
	return aio_socket_accept(s_webserver, OnAccept, NULL); // start server
}

int main(int argc, char* argv[])
{
#if defined(OS_LINUX)
	/* ignore pipe signal */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, 0);
	sigaction(SIGPIPE, &sa, 0);
#endif

	s_workers = system_getcpucount() * 2;

	// use proxy
	config_proxy_load();
	http_proxy_add_pattern("*.budejie.com");
	http_proxy_add_pattern("*.qiushibaike.com");
	http_proxy_add_pattern("*.baozoumanhua.com");
	http_proxy_add_pattern("*.yyxj8.com");

	socket_init();
	jokedb_init();
	jokecomment_init();

	g_taskQ = sys_task_queue_create(20); // task queue

	aio_socket_init(s_workers, 2*60*1000);
	for(int i=0; i<s_workers; i++)
		sys_thread_pool_push(OnWorker, NULL); // start worker

	InitWebServer(NULL, 2001); // start web server

	for(char c = getchar(); 'q' != c ; c = getchar())
	{
	}

	aio_socket_destroy(s_webserver);
	aio_socket_clean();
	sys_task_queue_destroy(g_taskQ);
	jokecomment_save();
	jokedb_clean();
	socket_cleanup();
	return 0;
}
