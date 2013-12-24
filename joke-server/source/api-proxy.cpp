#include "web-session.h"
#include "http-proxy.h"
#include "cppstringext.h"

int config_proxy_save();

int WebSession::OnProxy()
{
	char msg[128] = {0};
	const char* from = http_get_header_by_name(m_http, "x-forward-for");
	if(!from)
		from = m_ip.c_str();
	snprintf(msg, sizeof(msg), "Hello Proxy: %s", from);

	return Reply(std::string(msg));
}

int WebSession::OnAddProxy()
{
	std::string proxy;
	if(!m_params.Get("proxy", proxy))
		return Reply(ERROR_PARAM, "miss proxy");

	std::vector<std::string> proxies;
	std::vector<std::string>::iterator it;
	Split(proxy.c_str(), ',', proxies);
	for(it = proxies.begin(); it != proxies.end(); ++it)
	{
		std::string host = Strip(it->c_str());
		http_proxy_add(host.c_str());
	}

	config_proxy_save(); //save to file
	return Reply(0, "OK");
}

int WebSession::OnDelProxy()
{
	std::string proxy;
	if(!m_params.Get("proxy", proxy))
		return Reply(ERROR_PARAM, "miss proxy");

	std::vector<std::string> proxies;
	std::vector<std::string>::iterator it;
	Split(proxy.c_str(), ',', proxies);
	for(it = proxies.begin(); it != proxies.end(); ++it)
	{
		std::string host = Strip(it->c_str());
		http_proxy_delete(host.c_str());
	}

	config_proxy_save(); //save to file
	return Reply(0, "OK");
}

static void OnListProxyHelper(void* param, const char* value)
{
	jsonarray* json = (jsonarray*)param;
	json->add(value);
}

int WebSession::OnListProxy()
{
	jsonarray json;
	http_proxy_list(OnListProxyHelper, &json);

	std::string reply = json.json();
	return Reply(reply.c_str());
}
