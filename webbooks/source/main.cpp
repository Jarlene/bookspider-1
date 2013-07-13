#include <stdio.h>
#include <stdlib.h>
#include "sys/sock.h"
#include "bookmanager.h"

int g_param_bookall = 1;

int BookWorker();
int BookSpider();

int main(int argc, char* argv[])
{
	socket_init();

	BookManager* bookmgr;
	bookmgr = BookManager::FetchBookManager();

	//BookWorker();
	BookSpider();

	socket_cleanup();
	return 0;
}
