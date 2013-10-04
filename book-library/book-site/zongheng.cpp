#include "zongheng.h"

struct TopUrls
{
	int type;
	std::string url;
};

static TopUrls g_urls[] = { 
	{ ETT_ALL_VIEW, "http://book.zongheng.com/store/c0/c0/b9/u1/p%d/v9/s9/t0/ALL.html" },
	{ ETT_ALL_MARK, "http://book.zongheng.com/store/c0/c0/b9/u4/p%d/v9/s9/t0/ALL.html" },
	{ ETT_ALL_VOTE, "http://book.zongheng.com/store/c0/c0/b9/u2/p%d/v9/s9/t0/ALL.html" },
	{ ETT_MONTH_VIEW, "http://book.zongheng.com/store/c0/c0/b9/u6/p%d/v9/s9/t0/ALL.html" },
	{ ETT_MONTH_VOTE, "http://book.zongheng.com/store/c0/c0/b9/u7/p%d/v9/s9/t0/ALL.html" },
	{ ETT_WEEK_VIEW, "http://book.zongheng.com/store/c0/c0/b9/u9/p%d/v9/s9/t0/ALL.html" },
	{ ETT_WEEK_VOTE, "http://book.zongheng.com/store/c0/c0/b9/u10/p%d/v9/s9/t0/ALL.html" },
};

const char* CZongHeng::GetUri(int top) const
{
	for(size_t i=0; i<sizeof(g_urls)/sizeof(g_urls[0]); i++)
	{
		if(g_urls[i].type == top)
			return g_urls[i].url.c_str();
	}
	return NULL;
}
