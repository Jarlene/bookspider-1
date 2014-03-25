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
	function __construct($proxy)
	{
		function_exists('curl_init') || die('CURL Library Not Loaded');     //exit if don't install curl

		$this->m_proxies = split(",", file_get_contents($proxy));

		$this->m_options = array(
			'port' => 80,
			'timeout' => 20,
			'cookie' => false,
			'ssl' => false,
			'gzip' => true,
			'proxy' => false
		);

		$t = gettimeofday(true);
		$this->m_headers = array( // http headers
			"User-Agent: Mozilla/5.0 " . $t,
			'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
			'Accept-Encoding: gzip, deflate',
			'Accept-Language: en-US,en;q=0.5'
		);

		$this->m_multi = curl_multi_init();
	}

	function __destruct()
	{
		curl_multi_close($this->m_multi);
	}

	function get($uri, $pattern, $timeout=10, $headers=array())
	{
		return $this->_request("GET", $uri, null, $pattern, $timeout, $headers);
	}

	function post($uri, $data, $pattern, $timeout=10, $headers=array())
	{
		return $this->_request("POST", $uri, $data, $pattern, $timeout, $headers);
	}

	private function _request($method, $url, $data, $pattern, $timeout, $headers)
	{
		$j = time() * 6;

		$curls = array();
		for($i  = 0; $i < 5 && $i < count($this->m_proxies); $i++){
			$curl = curl_init($url);
			$this->_setopt($method, $curl, $data ? $data : null, $headers);

			// set proxy
			$host = $this->m_proxies[($j + $i) % count($this->m_proxies)];
			$proxy = split(":", $host);
			if(count($proxy) > 1){
				curl_setopt($curl, CURLOPT_PROXYPORT, $proxy[1]);
			} else {
				curl_setopt($curl, CURLOPT_PROXYPORT, 80);
			}
			curl_setopt($curl, CURLOPT_PROXY, $proxy[0]);
			curl_setopt($curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);

			curl_multi_add_handle($this->m_multi, $curl);
			$curls[] = $curl;
		}

		$r = $this->_perform($this->m_multi, $timeout, $pattern);

		foreach($curls as $curl){
			curl_multi_remove_handle($this->m_multi, $curl);
			curl_close($curl);
		}

		return $r;
	}

	private function _perform($multi, $timeout, $pattern)
	{
		// running
		do{
			$status = curl_multi_exec($multi, $active);
		} while($status === CURLM_CALL_MULTI_PERFORM);
			
		while(CURLM_OK === $status && $active > 0){
			if(curl_multi_select($multi, $timeout) < 1) {
				// log timeout
				return False;
			}
			
			do{
				$status = curl_multi_exec($multi, $active);
			} while($status === CURLM_CALL_MULTI_PERFORM);

			$v = $this->_read($multi, $pattern);
			if($v){
				return $v;
			}
		}

		if(CURLM_OK !== $status || 0 !== $active){
			// log
		}
		return False;
	}

	private function _read($multi, $pattern)
	{
		$info = curl_multi_info_read($multi);
		while($info){
			assert(CURLMSG_DONE === $info["msg"]);
			if(CURLE_OK === $info["result"]){
				$curl = $info["handle"];
				$html = curl_multi_getcontent($curl);
				$size = curl_getinfo($curl, CURLINFO_HEADER_SIZE);
				if($size > 0 && strlen($html) >= $size){
					//$head = substr($html, 0, $size);
					$body = substr($html, $size);
					if(stripos($body, $pattern)){
						return $body;
					}
				}
			}

			// next
			$info = curl_multi_info_read($multi);
		}

		return False;
	}

	private function _setopt($method, $curl, $data, $headers)
	{
		$options = $this->m_options;
		curl_setopt($curl, CURLOPT_PORT, $options['port']); // HTTP default port
		curl_setopt($curl, CURLOPT_FOLLOWLOCATION, true); // enable HTTP Location header
		//curl_setopt($curl, CURLOPT_USERAGENT, $this->m_headers['User-Agent']);  // default user-agent
		//curl_setopt($curl, CURLOPT_CONNECTTIMEOUT, $options['timeout']); // connection timeout
		//curl_setopt($curl, CURLOPT_TIMEOUT, $options['timeout']); // execute timeout
		curl_setopt($curl, CURLOPT_HEADER, true); // get HTTP header
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true); // get HTTP body
		curl_setopt($curl, CURLOPT_BINARYTRANSFER, true); // binary content
		curl_setopt($curl,CURLOPT_ENCODING, ''); // decode gzip

		// proxy
		// if($options['proxy']){
			// $proxyType = $options['proxyType']=='HTTP' ? CURLPROXY_HTTP : CURLPROXY_SOCKS5;
			// curl_setopt($curl, CURLOPT_PROXYTYPE, $proxyType);
			// curl_setopt($curl, CURLOPT_PROXY, $options['proxyHost']);
			// curl_setopt($curl, CURLOPT_PROXYPORT, $options['proxyPort']);

			// if($options['proxyAuth']){
				// $proxyAuthType = $options['proxyAuthType']=='BASIC' ? CURLAUTH_BASIC : CURLAUTH_NTLM;  
                // $proxyUser = "[{$options['proxyAuthUser']}]:[{$options['proxyAuthPwd']}]";  
				// curl_setopt($curl, CURLOPT_PROXYAUTH, $proxyAuthType);  
                // curl_setopt($curl, CURLOPT_PROXYUSERPWD, $proxyUser);  
			// }
		// }

		// ssl
		if($options['ssl']){
			curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
			curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, true);
		}

		// cookie
		if($options['cookie']){
			$cookfile = tempnam(sys_get_temp_dir(), 'curl-cookie-');
			curl_setopt($curl, CURLOPT_COOKIEJAR, $cookfile);
            curl_setopt($curl, CURLOPT_COOKIEFILE, $cookfile);
		}
	
		foreach($this->m_headers as $header){
			$headers[] = $header;
		}

//		curl_setopt($curl, CURLOPT_URL, $uri);
		curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);

		if($method == "POST" && $data){
			curl_setopt($curl, CURLOPT_POST, true);
			curl_setopt($curl, CURLOPT_POSTFIELDS, $data);	
		}
	}

	private $m_multi = null;
	private $m_proxies = null;
	private $m_options = null;
	private $m_headers = null;
}

//print_r(http_proxy_get("http://www.pingshu8.com/play_23925.html", "luckyzz@163.com"));
?>
