#include "cstringext.h"
#include "sys/sock.h"
#include <stdio.h>
#include "bookmanager.h"

#include "17k.h"
#include "qidian.h"
#include "zongheng.h"

int BookTop();
int BookLibrary();

int main(int argc, char **argv)
{
	socket_init();

	book_info book;
	std::string chapter;
	CZongHeng zongheng;
	read_index(&zongheng, "http://book.zongheng.com/showchapter/48552.html", NULL, book);
	zongheng.ReadChapter("http://book.zongheng.com/chapter/39813/3751818.html", chapter);

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
