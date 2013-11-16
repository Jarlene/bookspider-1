#include "17k.h"

struct TopUrls
{
	int type;
	std::string url;
};

static TopUrls g_urls[] = { 
	{ ETT_ALL_VIEW, "http://all.17k.com/all/0_0_sc_0_1_%d.html" },
	{ ETT_ALL_MARK, "http://all.17k.com/all/0_0_fs_0_1_%d.html" },
	{ ETT_ALL_VOTE, "http://all.17k.com/all/0_0_fs_0_1_%d.html" }, // miss vote, instead of mark
	{ ETT_MONTH_VIEW, "http://all.17k.com/all/0_0_mc_0_1_%d.html" },
	{ ETT_MONTH_MARK, "http://all.17k.com/all/0_0_fm_0_1_%d.html" },
	{ ETT_MONTH_VOTE, "http://all.17k.com/all/0_0_fm_0_1_%d.html" },
	{ ETT_WEEK_VIEW, "http://all.17k.com/all/0_0_wc_0_1_%d.html" },
	{ ETT_WEEK_MARK, "http://all.17k.com/all/0_0_fw_0_1_%d.html" },
	{ ETT_WEEK_VOTE, "http://all.17k.com/all/0_0_fw_0_1_%d.html" },
};

const char* C17K::GetUri(int top) const
{
	for(size_t i=0; i<sizeof(g_urls)/sizeof(g_urls[0]); i++)
	{
		if(g_urls[i].type == top)
			return g_urls[i].url.c_str();
	}
	return NULL;
}

int C17K::ReadBook(const char* uri, book_info& book)
{
	return 0;
}

int C17K::ReadChapter(const char* uri, std::string& chapter)
{
	return 0;
}
