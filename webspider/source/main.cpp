#include <stdio.h>
#include "sys/sock.h"

int WebSpider(const char* spider);

static void PrintHelpAndExit()
{
	printf("webspider 0.1\n");
	printf("webspider [-h|--heper|--spider name]\n");
	exit(0);
}

int main(int argc, char* argv[])
{
	const char* spider = NULL;
	for(int i=1; i<argc; i++)
	{
		if(0==strcmp("-h", argv[i]) || 0==strcmp("--help", argv[i]))
		{
			PrintHelpAndExit();
		}
		else if(0 == strcmp("--spider", argv[i]))
		{
			if(i+1 >= argc)
			{
				PrintHelpAndExit();
			}
			spider = argv[++i];
		}
	}

	socket_init();
	WebSpider(spider);
	socket_cleanup();
	return 0;
}
