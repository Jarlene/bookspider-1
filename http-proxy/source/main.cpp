#include "http.h"
#include "sys/sock.h"
#include "time64.h"
#include "http-proxy.h"
#include <stdio.h>
#include <stdlib.h>

/*
static void http_proxy_keepalive(sys_timer_t id, void* param)
{
	std::vector<TProxyKeepAlive> proxies;
	{
		TProxies::iterator it;
		AutoThreadLocker locker(s_locker);
		for(it = s_proxies.begin(); it != s_proxies.end(); ++it)
		{
			TProxyKeepAlive proxy;
			memset(&proxy, 0, sizeof(TProxyKeepAlive));
			strcpy(proxy.proxy, it->proxy);
			proxies.push_back(proxy);
		}
	}

	int r, port;
	host_t host;
	for(size_t i=0; i<proxies.size(); i++)
	{
		host_parse(proxies[i].proxy, host, &port);

		proxies[i].socket = socket_tcp();
		r = socket_setnonblock(proxies[i].socket, 1);
		r = socket_connect_ipv4(proxies[i].socket, host, (unsigned short)port);
		assert(r <= 0);
		if(0==r || WSAEWOULDBLOCK==WSAGetLastError())
		{
			//FD_SET(sockets[i], &fds);
		}
		else
		{
			socket_close(proxies[i].socket);
			proxies[i].socket = 0;
		}

		proxies[i].delay = -1;
	}

	time64_t t0 = time64_now();
	time64_t tnow = t0;
	do
	{
		fd_set fds;
		FD_ZERO(&fds);
		socket_t socket = 0;
		for(size_t i=0; i<proxies.size(); i++)
		{
			socket = max(proxies[i].socket, socket);
			if(0 != proxies[i].socket)
				FD_SET(proxies[i].socket, &fds);
		}

		int timeout = TIMEOUT-(int)(tnow - t0);

		struct timeval tv;
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000) * 1000;
		r = socket_select_writefds(socket+1, &fds, &tv);

		tnow = time64_now();
		for(size_t i = 0; i < proxies.size(); i++)
		{
			if(FD_ISSET(proxies[i].socket, &fds))
				proxies[i].delay = (int)(tnow - t0);
		}
	} while(r > 0 && t0 + TIMEOUT <= tnow);

	printf("==============Proxy Keep-Alive==============\n");
	for(size_t i = 0; i < proxies.size(); i++)
	{
		if(proxies[i].delay >= 0)
			printf("%s - %d\n", proxies[i].proxy, proxies[i].delay);

		if(0 != proxies[i].socket)
		{
			socket_close(proxies[i].socket);
			http_proxy_set_delay(proxies[i].proxy, proxies[i].delay);
		}
	}
}
*/

int main(int argc, char* argv[])
{
	http_proxy_add_pattern("115.28.51.131");

	socket_init();
	const char* proxies[] = { 
		//"59.151.37.8:8028",
		//"59.151.88.241:80",
		"14.18.16.67:80",
		"14.18.16.68:80",
		"14.18.16.69:80",
		"14.18.16.70:80",
		"61.55.141.10:81",
		"61.55.141.11:81",
		//"61.158.219.226:8118",
		//"61.163.163.145:9999",
		//"61.156.235.172:9999",
		//"61.156.235.173:9999",
		//"61.153.236.30:80",
		//"61.153.236.30:8080",
		//"101.4.60.101:80",
		//"101.4.60.193:80",
		//"101.4.60.202:80",
		//"101.4.60.203:80",
		//"103.16.26.78:80",
	};
	for(int i=0; i<sizeof(proxies)/sizeof(proxies[0]); i++)
	{
		http_proxy_add(proxies[i]);

		time64_t t1 = time64_now();
		void* reply = NULL;
		int r = http_request("http://115.28.51.131/joke/api/proxy", "", &reply);
		time64_t t2 = time64_now();
		if(0 == r)
		{
			printf("[%u][%s]: %s\n", (unsigned int)(t2-t1), proxies[i], (char*)reply);
			free(reply);
		}

		http_proxy_delete(proxies[i]);
	}

	return 0;
}
