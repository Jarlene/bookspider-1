#include "book-site.h"
#include "cstringext.h"
#include "sys/process.h"

#include "17k.h"
#include "qidian.h"
#include "zongheng.h"

static int OnBook(void* param, const book_t* book)
{
	BookManager* bookmgr = BookManager::FetchBookManager();
	if(!bookmgr)
	{
		printf("%s:%d fetch book manager failed.\n", __FILE__, __LINE__);
		return 0;
	}

	int r = bookmgr->AddBook(*book);
	if(r < 0)
	{
		printf("%s:%d add book %s failed: %d\n", __FILE__, __LINE__, book->name, r);
		return r;
	}
	else if(0 == r)
	{
		// add book
		//InterlockedIncrement(&g_bookstatistic.newbooks);
	}
	else if(1 == r)
	{
		// update book
		//InterlockedIncrement(&g_bookstatistic.updatebooks);
	}
	else
	{
		assert(false);
	}

	int *n = (int*)param;
	return --*n > 0 ? 0 : 1;
}

static int OnThread(void* param)
{
	IBookSite* site = (IBookSite*)param;
	EBookTop types[] = { ETT_ALL_VIEW, ETT_ALL_MARK, ETT_ALL_VOTE, };
	for(int i=0; i<sizeof(types)/sizeof(types[0]); i++)
	{
		int n = site->GetCount();;
		int r = ListBook(site, types[i], OnBook, &n);
		if(r < 0)
		{
			printf("BookTopWorker[%d] error: %d\n", i, r);
			continue;
		}
	}

	return 0;
}

int BookLibrary()
{
	IBookSite* sites[] = { new CQiDian(),  new CZongHeng(), new C17K(), };

	// start worker thread
	std::vector<thread_t> threads;
	for(int i=0; i<sizeof(sites)/sizeof(sites[0]); i++)
	{
		thread_t thread;
		thread_create(&thread, OnThread, sites[i]);
		threads.push_back(thread);
	}

	// wait for all thread exit
	for(size_t i=0; i<threads.size(); i++)
	{
		thread_destroy(threads[i]);
	}

	return 0;
}
