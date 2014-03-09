<?php
class HttpMultipleProxy
{
	function __construct($file)
	{
		$this->m_file = $file;
		$content = file_get_contents($file);
		$this->m_proxies = split(",", $content);
		srand(time());
	}

	function __destruct()
	{
		//file_put_contents($this->m_file, implode(",", $this->m_proxies));
	}
	
	function get($urls, $callback, &$param, $timeout=20, $headers=array())
	{
		$this->m_callback = $callback;
		$this->m_param = $param;

		for($i = 0; $i < count($urls); $i++){
			$this->m_urls[] = array("uri" => $urls[$i], "data" => null);
		}

		return $this->_requestX("GET", $timeout, $headers);
	}

	function post($urls, $data, $callback, &$param, $timeout=20, $headers=array())
	{
		$this->m_callback = $callback;
		$this->m_param = $param;

		if($data && count($data) != count($urls)){
			return -1;
		}

		for($i = 0; $i < count($urls); $i++){
			$this->m_urls[] = array("uri" => $urls[$i], "data" => ($data ? $data[$i] : null));
		}

		return $this->_requestX("POST", $timeout, $headers);
	}

	private function _requestX($method, $timeout, $headers)
	{
		for($i = 0; $i < 5 && 0 != count($this->m_urls); $i++)
		{
			$r = $this->_request($method, 5, $headers);
		}

		foreach($this->m_urls as $k => $v){
			if(is_callable($this->m_callback)) {
				call_user_func($this->m_callback, &$this->m_param, $k, -1, "", "");
			}
		}

		return 0 == count($this->m_urls) ? 0 : -1;
	}

	private function _request($method, $timeout, $headers)
	{
		$proxies = array();
		$this->m_iProxy = rand();
		for($i = 0; $i < count($this->m_urls) && $i < count($this->m_proxies); $i++){
			$j = ($i + $this->m_iProxy) % count($this->m_proxies);
			$proxies[] = $this->m_proxies[$j];
		}

		$urls = array();
		$data = array();
		$this->m_mapUrls = array();
		foreach($this->m_urls as $k => $v){
			$urls[] = $v["uri"];
			$data[] = $v["data"];
			$this->m_mapUrls[] = $k;
		}

		$result = array();
		$http = new HttpMultiple();
		$http->setproxy($proxies);		
		if(0 == strcmp("POST", $method)){
			$r = $http->post($urls, $data, array($this, '_on_request'), $result, $timeout, $headers);
		} else {
			$r = $http->get($urls, array($this, '_on_request'), $result, $timeout, $headers);
		}

		if(0 == count($result)){
			// all proxy connection failed, so use other proxies
			for($i = 0; $i < count($this->m_urls) && $i < count($this->m_proxies); $i++){
				$j = ($i + $this->m_iProxy) % count($this->m_proxies);
				unset($this->m_proxies[$j]);
			}
			$this->m_proxies = array_values($this->m_proxies);
		} else {
			$this->m_proxies = $result; // update proxy list	
		}
		
		return $r;
	}

	// callback function can't be private
	function _on_request(&$proxies, $i, $r, $header, $body)
	{
		if(0 == $r){
			$idx = $this->m_mapUrls[$i];
			if(is_callable($this->m_callback)) {
				$r = call_user_func($this->m_callback, &$this->m_param, $idx, $r, $header, $body);
			}

			if(0 == $r){
				unset($this->m_urls[$idx]); // work done, remove from work list

				if(count($this->m_proxies) > 0){
					$proxy = ($i + $this->m_iProxy) % count($this->m_proxies);
					$proxies[] = $this->m_proxies[$proxy]; // reuse validate proxy
				}
			}
		}
	}

	private $m_urls = null;
	private $m_mapUrls = null;
	private $m_file = null;
	private $m_iProxy = 0;
	private $m_proxies = null;
	private $m_cbparam = null;
	private $m_callback = null;
};

// require("php/http-multiple.inc");
// function OnReadData($param, $idx, $r, $header, $body)
// {
	// print_r("onreaddata $idx : $r\r\n");
	// if(strpos($body, "2011-2014, YOUDAILI.CN, Inc. All Rights Reserved")){	
		// print_r("onreaddata $idx : ok\r\n");
		// return 0;
	// }
	// return -1;
// }

// $urls = array("http://www.youdaili.cn/Daili/guonei/1773.html");
// $dir = dirname($urls[0]);
// $name = basename($urls[0], ".html");
// for($i = 2; $i <= 5; $i++){
	// $urls[] = "$dir/$name" . "_$i.html";
// }

// $proxies = array();
// $http = new HttpMultipleProxy("proxy.cfg");
// $http->get($urls, 'OnReadData', &$proxies);
// print_r(count($proxies) . "\r\n");
?>
