<?php
class HttpMultiple
{
	function __construct($options=array())
	{
		$this->m_multi = curl_multi_init();
		$this->m_proxies = array();
		$this->m_options = array(
			'port' => 80,
			'timeout' => 20,
			'cookie' => false,
			'ssl' => false,
			'gzip' => true,
			'proxy' => false
		);
		$this->m_headers = array( // http headers
			'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:26.0) Gecko/20100101 Firefox/26.0',
			'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
			'Accept-Encoding: gzip, deflate',
			'Accept-Language: en-US,en;q=0.5'
		);
	}

	function __destruct()
	{
		curl_multi_close($this->m_multi);
	}

	function get($urls, $callback, $param, $timeout=20, $headers=array())
	{
		return $this->_request("GET", $urls, null, $callback, $param, $timeout, $headers);
	}

	function post($urls, $data, $callback, $param, $timeout=20, $headers=array())
	{
		if(count($urls) != count($data))
			return -1;

		return $this->_request("POST", $urls, $data, $callback, $param, $timeout, $headers);
	}

	// setproxy("1.1.36.27", "120.197.85.173:33942", "120.197.85.182:10080", "221.130.162.242:82", "221.130.162.242:9000")
	function setproxy($hosts)
	{
		$this->m_proxies = array();

		$proxies = array();
		foreach($hosts as $host){
			$proxy = split(":", $host);
			if(count($proxy) > 1){
				$this->m_proxies[] = array("proxy" => CURLPROXY_HTTP, "host" => $proxy[0], "port" => $proxy[1]);
			} else {
				$this->m_proxies[] = array("proxy" => CURLPROXY_HTTP, "host" => $proxy[0], "port" => 80);
			}
		}
	}

	private function _request($method, $urls, $data, $callback, $param, $timeout, $headers)
	{
		$pxidx = 0;
		//$proxies = http_get_proxies();
		assert(count($urls) == count($data));

		$curls = array();
		for($i=0; $i < count($urls); $i++){
			$curl = curl_init($urls[$i]);
			$this->_setopt($method, $curl, $data[$i], $headers);

			if(count($proxies) > 0){
				$idx = $pxidx++ % count($proxies);
				curl_setopt($curl, CURLOPT_PROXY, $proxies[$idx]['host']);
				curl_setopt($curl, CURLOPT_PROXYPORT, $proxies[$idx]['port']);
				curl_setopt($curl, CURLOPT_PROXYTYPE, $proxies[$idx]['proxy']);
			}

			curl_multi_add_handle($multi, $curl);
			$curls[] =  $curl;
		}

		$r = $this->_perform($this->m_multi, $curls, $timeout, $callback, $param);
		if(0 != $r){
			assert(0 != count($curls), "HttpMultiple _perform error.");

			// disable proxy
			if(count($proxies) > 0){
				foreach($curls as $curl){
					curl_setopt($curl, CURLOPT_PROXYPORT, "");
				}
				$r = $this->_perform($this->m_multi, $curls, $timeout, $callback, $param);
			}
		} else {
			assert(0 == count($curls), "HttpMultiple _perform error.");
		}

		// close
		foreach($curls as $curl){
			curl_multi_remove_handle($this->m_multi, $curl);
			curl_close($curl);
		}

		return $r;
	}

	private function _setopt($method, $curl, $data, $headers)
	{
		function_exists('curl_init') || die('CURL Library Not Loaded');     //exit if don't install curl

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

		if($method == "POST"){
			curl_setopt($curl, CURLOPT_POST, true);
			curl_setopt($curl, CURLOPT_POSTFIELDS, $data);	
		}
	}

	private function _perform($multi, $curls, $timeout, $callback, $param)
	{
		$t0 = gettimeofday(true);

		// running
		do{
			$status = curl_multi_exec($multi, $active);
		} while($status === CURLM_CALL_MULTI_PERFORM);

		while($status === CURLM_OK && $active){
			$t1 = gettimeofday(true);
			if(0 != $timeout && $t0 + $timeout < $t1){
				return -1;
			}

			$t = $timeout - ($t1 - $t0);
			if(curl_multi_select($multi, -1==$timeout?-1:$t) < 1) {
				return -1;
			}

			$info = curl_multi_info_read($multi);
			while($info){
				$curl = $info['handle'];
				$idx = array_search($curl, $curls, true);
				$response = curl_multi_getcontent($curl);
				$headerSize = curl_getinfo($curl, CURLINFO_HEADER_SIZE);
				$header = substr($response, 0, $headerSize);
				$body = substr($response, $headerSize);
				if (is_callable($callback)) {
					call_user_func($callback, $param, $header, $body, $idx);
				}
				curl_multi_remove_handle($multi, $curl);
				unset($curls[$idx]);
				curl_close($curl);

				//ob_flush();flush();

				$info = curl_multi_info_read($multi);
			}

			do{
				$status = curl_multi_exec($multi, $active);
			} while($status === CURLM_CALL_MULTI_PERFORM);
		}

		$t1 = gettimeofday(true);
		echo "time: " . ($t1-$t0) . "\r\n";
		return ($status === CURLM_OK && $active) ? 0 : -1;
	}

	private $m_multi = null;
	private $m_proxies = null;
	private $m_options = null;
	private $m_headers = null;
};

function OnReadData($param, $header, $body, $idx)
{
	file_put_contents($idx . ".html", $body);
	//echo 'OnReadData[' . $idx . '] ' . strlen($content);
}

$urls = array("http://www.baidu.com/", "http://www.bing.com/", "http://www.google.com/");
http_get_multiple($urls, 'OnReadData', "param", -1);

// print_r($all);
//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm'));
//print_r(ysts8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
