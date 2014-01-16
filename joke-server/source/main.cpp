#include "sys/sock.h"
#include "sys/system.h"
#include "aio-socket.h"
#include "sys-thread-pool.h"
#include "sys-task-queue.h"
#include "sys-timer.h"
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

int main(int argc, char* argv[])
{
#if defined(OS_LINUX)
	/* ignore pipe signal */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, 0);
	sigaction(SIGPIPE, &sa, 0);
#endif

	int port = 2001;
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

	WebSession::Start(NULL, port); // start web server
	for(char c = getchar(); 'q' != c ; c = getchar())
	{
	}

	WebSession::Stop();
	aio_socket_clean();
	sys_task_queue_destroy(g_taskQ);
	jokecomment_save();
	jokedb_clean();
	socket_cleanup();
	return 0;
}
