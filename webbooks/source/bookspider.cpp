#include "sys/process.h"
#include "sys/system.h"
#include "bookspider.h"
#include "bookmanager.h"
#include "utf8.h"
#include <stdio.h>
#include <vector>

inline std::string utf8_to_local(const char* utf8)
{
	char book[512];
	utf8_to_gb18030(utf8, book, sizeof(book));
	return book;
}

static int BookSpiderWorker(void* param)
{
	char bookuri[128];
	char indexuri[128];
	const book_spider_t* spider = book_spider_get((int)param);

	BookManager* bookmgr = BookManager::FetchBookManager();
	
	//int mid = bookmgr->GetBookSiteMid(spider->name);
	int mid = 0;

	for(int i=0; bookmgr; i)
	{
		std::vector<std::pair<int, BookManager::Book> > books;
		bookmgr->QueryBook(mid, i, 100, books);
		if(!books.size())
			break;
		i += books.size();

		for(int j=0; j<(int)books.size(); j++)
		{
			int bookid = books[j].first;
			if(bookmgr->CheckBookSite(bookid, spider->name))
				continue; // exist

			const BookManager::Book& book = books[j].second;
			int r = spider->search(book.name, book.author, bookuri, indexuri);
			if(r < 0)
			{
				printf("%s[%d/%d]: %s error %d\n", spider->name, j, books.size(), utf8_to_local(book.name).c_str(), r);
				continue;
			}

			printf("%s[%d/%d]: %s->%s\n", spider->name, j, books.size(), utf8_to_local(book.name).c_str(), bookuri);
			bookmgr->AddBookSite(bookid, spider->name, bookuri, indexuri);

			system_sleep(spider->interval);
		}
	}

	return 0;
}

int BookSpider()
{
	// start worker thread
	std::vector<thread_t> threads;
	for(int i=0; i<book_spider_count(); i++)
	{
		thread_t thread;
		thread_create(&thread, BookSpiderWorker, (void*)i);
		threads.push_back(thread);
	}

	// wait for all thread exit
	for(size_t i=0; i<threads.size(); i++)
	{
		thread_destroy(threads[i]);
	}
	return 0;
}
