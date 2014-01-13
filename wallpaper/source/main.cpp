#include "sys/sock.h"
#include "sys/system.h"
#include "tcpserver.h"
#include "aio-socket.h"
#include "sys-thread-pool.h"
#include "sys-task-queue.h"
#include "sys-timer.h"
#include "web-session.h"
#include "http-proxy.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef OS_LINUX
#include <signal.h>
#endif

static int s_workers = 0;
static aio_socket_t s_webserver;
sys_task_queue_t g_taskQ;

int config_proxy_load();

void AioWorker(void* param)
{
	while(1)
	{
		int r = aio_socket_process(2*60*1000);
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

	printf("server listen at %s:%d\n", ip?ip:"localhost", port);
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

	int port = 2002;
	for(int i=1; i<argc; i++)
	{
		if(streq(argv[i], "--port") && i+1<argc)
		{
			port = atoi(argv[++i]);
		}
	}

	// use proxy
	config_proxy_load();
	http_proxy_add_pattern("*.budejie.com");
	http_proxy_add_pattern("*.qiushibaike.com");
	http_proxy_add_pattern("*.baozoumanhua.com");
	http_proxy_add_pattern("*.yyxj8.com");

	socket_init();
	jokedb_init();
	jokecomment_init();

	s_workers = system_getcpucount() * 2;
	g_taskQ = sys_task_queue_create(s_workers); // task queue

	aio_socket_init(s_workers);
	for(int i=0; i<s_workers; i++)
		sys_thread_pool_push(AioWorker, NULL); // start worker

	InitWebServer(NULL, port); // start web server

	WebSession::OnJokeTimer(NULL, NULL);
	WebSession::On18PlusTimer(NULL, NULL);
	sys_timer_t jokeTimer = NULL;
	sys_timer_t comicTimer = NULL;
	sys_timer_start(&jokeTimer, 15*60*1000, WebSession::OnJokeTimer, NULL);
	sys_timer_start(&comicTimer, 15*60*1000, WebSession::On18PlusTimer, NULL);
	for(char c = getchar(); 'q' != c ; c = getchar())
	{
	}

	sys_timer_stop(&jokeTimer);
	sys_timer_stop(&comicTimer);
	aio_socket_destroy(s_webserver);
	aio_socket_clean();
	sys_task_queue_destroy(g_taskQ);
	jokecomment_save();
	jokedb_clean();
	socket_cleanup();
	return 0;
}
