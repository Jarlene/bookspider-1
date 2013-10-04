#include "qidian.h"

struct TopUrls
{
	int type;
	std::string url;
};

static TopUrls g_urls[] = { 
	{ ETT_ALL_VIEW, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=13&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_ALL_MARK, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=9&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_ALL_VOTE, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=3&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_MONTH_VIEW, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=12&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_MONTH_VOTE, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=4&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_WEEK_VIEW, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=11&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
	{ ETT_WEEK_VOTE, "http://all.qidian.com/book/bookStore.aspx?ChannelId=-1&SubCategoryId=-1&Tag=all&Size=-1&Action=-1&OrderId=8&P=all&PageIndex=%d&update=-1&Vip=-1&Boutique=-1&SignStatus=-1" },
};

const char* CQiDian::GetUri(int top) const
{
	for(size_t i=0; i<sizeof(g_urls)/sizeof(g_urls[0]); i++)
	{
		if(g_urls[i].type == top)
			return g_urls[i].url.c_str();
	}
	return NULL;
}
