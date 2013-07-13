#include "booksite.h"
#include <vector>

typedef std::vector<const book_site_t*> sites_t;
static sites_t g_sites;

int book_site_register(const book_site_t* site)
{
	g_sites.push_back(site);
	return 0;
}

int book_site_count()
{
	return g_sites.size();
}

const book_site_t* book_site_get(int index)
{
	if(index >= book_site_count() || index<0)
		return NULL;

	return g_sites[index];
}
