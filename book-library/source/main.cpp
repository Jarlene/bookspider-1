#include "cstringext.h"
#include "sys/sock.h"
#include <stdio.h>

int BookTop();
int BookLibrary();

int main(int argc, char **argv)
{
	socket_init();

	for(int i=1; i<argc; i++)
	{
		if(streq(argv[i], "--top"))
		{
			BookTop();
		}
		else if(streq(argv[i], "--library"))
		{
			BookLibrary();
		}
		else
		{
			printf("Book-Library [--top | --library]\n");
			break;
		}
	}

	socket_cleanup();
	return 0;
}
