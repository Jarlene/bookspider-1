#include "bookspider.h"
#include <vector>

typedef std::vector<const book_spider_t*> spiders_t;
static spiders_t g_spiders;

int book_spider_register(const book_spider_t* spider)
{
	g_spiders.push_back(spider);
	return 0;
}

int book_spider_count()
{
	return g_spiders.size();
}

const book_spider_t* book_spider_get(int index)
{
	if(index >= book_spider_count() || index<0)
		return NULL;

	return g_spiders[index];
}
