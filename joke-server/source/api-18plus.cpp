#include "web-session.h"
#include "joke-db.h"

#define MAX_PAGE 50

int WebSession::On18Plus()
{
	int page = 0;
	int limit = 50;
	std::string tseq;
	m_params.Get("s", tseq);
	m_params.Get("page", page);
	m_params.Get("limit", limit);

	if(page > MAX_PAGE)
		return ReplyRedirectTo("/joke/");

	return 0;
}
