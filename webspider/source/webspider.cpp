#include "cstringext.h"
#include "sys/process.h"
#include "sys/system.h"
#include "bookspider.h"
#include "bookmanager.h"
#include "ChapterManager.h"
#include <list>
#include <vector>
#include "utf8.h"
#include "mmptr.h"

int ReadBook(const char* uri, const char* xml, std::list<std::string>& chapters);
int ReadChapter(const char* filename, std::list<std::string>& chapters);

static int WebSpiderWorker(void* param)
{
	const book_spider_t* spider = book_spider_get((int)param);

	BookManager* bookmgr = BookManager::FetchBookManager();

	for(int i=0; bookmgr; i)
	{
		std::vector<BookManager::BookSite> books;
		bookmgr->QueryBookFromSite(spider->name, i, 100, books);
		if(!books.size())
			break;
		i += books.size();

		for(int j=0; j<(int)books.size(); j++)
		{
			std::list<std::string> chapters;
			const BookManager::BookSite& book = books[j];
			int r = ReadBook(book.uri, spider->index, chapters);
			if(r)
			{
				printf("%s[%d/%d]: bid: %d error %d\n", spider->name, j, books.size(), book.bookid, r);
				continue;
			}

			if(chapters.size() < 3)
			{
				system_sleep(1);
				continue;
			}

			const std::string& chapter = *chapters.rbegin();
			mmptr ptr(chapter.length()+1);
			utf8_to_gb18030(chapter.c_str(), ptr, ptr.capacity());
			printf("%s[%d/%d]: bid %d->%s\n", spider->name, j, books.size(), book.bookid, (const char*)ptr);
			
			// update book site chapter
			r = bookmgr->SetBookSiteChapter(spider->name, book.bookid, chapters.rbegin()->c_str());
			if(r)
			{
				printf("%s[%d/%d]: bid: %d update book site chapter error: %d\n", spider->name, j, books.size(), book.bookid, r);
			}

			// check it the latest chapter
			r = ChapterManager::GetInstance().Update(book.bookid, chapters);
			if(r)
			{
				printf("%s[%d/%d]: bid: %d update book site chapter error: %d\n", spider->name, j, books.size(), book.bookid, r);
			}

			system_sleep(1);
		}
	}

	return 0;
}

int WebSpider(const char* spider)
{
	BookManager* bookmgr = BookManager::FetchBookManager();
	bookmgr;

	//std::list<std::string> chapters;
	//ReadChapter("E:\\app\\web\\output.xml", chapters);
	//ChapterManager::GetInstance().Update(2, chapters);

	// start worker thread
	std::vector<thread_t> threads;
	for(int i=0; i<book_spider_count(); i++)
	{
		const book_spider_t* item = book_spider_get(i);
		if(spider && *spider && 0!=strcmp(item->name, spider))
			continue;

		thread_t thread;
		thread_create(&thread, WebSpiderWorker, (void*)i);
		threads.push_back(thread);
	}

	// wait for all thread exit
	for(size_t i=0; i<threads.size(); i++)
	{
		thread_destroy(threads[i]);
	}
	return 0;
}
