<?php
function http_proxy_get($uri, $pattern, $timeout=20, $cfg="proxy.cfg", $headers=array())
{
	$http = new HttpProxy($cfg);
	return $http->get($uri, $pattern, $timeout, $headers);
}

function http_proxy_post($uri, $data, $pattern, $timeout=20, $cfg="proxy.cfg", $headers=array())
{
	$http = new HttpProxy($cfg);
	return $http->post($uri, $data, $pattern, $timeout, $headers);
}

class HttpProxy
{
	function __construct($proxy="proxy.cfg")
	{
		$this->m_proxies = split(",", file_get_contents("proxy.cfg"));
	}
	
	function get($uri, $pattern, $timeout=20, $headers=array())
	{
		return $this->_request("GET", $uri, null, $pattern, $timeout, $headers);
	}
	
	function post($uri, $data, $pattern, $timeout=20, $headers=array())
	{
		return $this->_request("POST", $uri, $data, $pattern, $timeout, $headers);
	}

	private function _request($method, $url, $data, $pattern, $timeout, $headers)
	{
		$base = time() * 6;

		$urls = array();
		$datum = array();
		$proxies = array();
		for($i  = 0; $i < 6 && $i < count($this->m_proxies); $i++){
			$urls[] = $url;
			$datum[] = $data;
			$proxies[] = $this->m_proxies[($base + $i) % count($this->m_proxies)];
		}

		$result = array();
		$http = new HttpMultiple();
		$http->setproxy($proxies);
		if($method == "POST"){
			$r = $http->post($urls, $datum, array($this, '__OnHttp'), &$result, $timeout, $headers);
		} else {
			$r = $http->get($urls, array($this, '__OnHttp'), &$result, $timeout, $headers);
		}

		foreach($result as $v){
			if(stripos($v, $pattern)){
				return $v;
			}
		}
		return "";
	}

	function __OnHttp($result, $idx, $r, $header, $body)
	{
		if(0 == $r){
			$result[] = $body;
		}
	}
	
	private $m_proxies = null;
}
?>
