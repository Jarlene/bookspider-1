#include "book-site.h"
#include "cstringext.h"
#include "sys/process.h"
#include "bookmanager.h"

#include "17k.h"
#include "qidian.h"
#include "zongheng.h"

#define MAX_TOP_BOOK	500

#define TOP_BOOK_STORE_LOCAL 1

#if TOP_BOOK_STORE_LOCAL
#include "../libsdk/json/jsonhelper.h"
#include "time64.h"
static void SaveBook(IBookSite* site, EBookTop type, const BookManager::Books& books)
{
	jsonarray jarr;
	BookManager::Books::const_iterator it;
	for(it = books.begin(); it != books.end(); ++it)
	{
		const BookManager::Book& book = *it;
		assert(0 != book.id);

		jsonobject json;
		json.add("name", book.name);
		json.add("author", book.author);
		json.add("category", book.category);
		json.add("count", book.vote);
		jarr.add(json);
	}

	char date[12] = {0};
	time64_format(time64_now(), "%04Y-%02M-%02D", date);
	char filename[128]= {0};
	snprintf(filename, sizeof(filename), "%s-%d-%s.json", site->GetName(), type, date);

	FILE* fp = fopen(filename, "w");
	std::string json = jarr.json();
	fwrite(json.c_str(), json.length(), 1, fp);
	fclose(fp);
}
#endif

static int OnBook(void* param, const book_t* book)
{
	BookManager::Books* books = (BookManager::Books*)param;
	books->push_back(*book);

	// top books
	return books->size() >= MAX_TOP_BOOK ? 1 : 0;
}

static int STDCALL OnThread(void* param)
{
	IBookSite* site = (IBookSite*)param;
	BookManager* bookmgr = BookManager::FetchBookManager();
	if(!bookmgr)
	{
		printf("%s:%d fetch book manager failed.\n", __FILE__, __LINE__);
		return 0;
	}

	EBookTop types[] = {ETT_MONTH_VIEW, /*ETT_MONTH_MARK, */ETT_MONTH_VOTE};
	for(int i=0; i<sizeof(types)/sizeof(types[0]); i++)
	{
		BookManager::Books books;
		int r = ListBook(site, types[i], OnBook, &books);
		if(r < 0)
		{
			printf("BookTopWorker[%d] error: %d\n", i, r);
			continue;
		}

#if TOP_BOOK_STORE_LOCAL
		SaveBook(site, types[i], books);
#else
		r = bookmgr->SetTopBooks(types[i], books);
		if(r < 0)
		{
			printf("%s:%d save site[%s], top[%d] books error: %d\n", __FILE__, __LINE__, site->GetName(), i, r);
			continue;
		}
#endif
	}

	return 0;
}

int BookTop()
{
	IBookSite* sites[] = { new CQiDian(), new CZongHeng(), new C17K() };

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
