<?php
class HttpMultiple
{
	function __construct($options=array())
	{
		function_exists('curl_init') || die('CURL Library Not Loaded');     //exit if don't install curl

		$this->m_proxies = array();
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
			"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:27.0) Gecko/20100101 Firefox/27.0",
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

	function get($urls, $callback, &$param, $timeout=10, $headers=array())
	{
		return $this->_request("GET", $urls, null, $callback, $param, $timeout, $headers);
	}

	function post($urls, $data, $callback, &$param, $timeout=10, $headers=array())
	{
		if(count($urls) != count($data))
			return -1;

		return $this->_request("POST", $urls, $data, $callback, $param, $timeout, $headers);
	}

	// setproxy(array("1.1.36.27", "120.197.85.173:33942", "120.197.85.182:10080", "221.130.162.242:82", "221.130.162.242:9000"))
	function setproxy($hosts)
	{
		$this->m_proxies = $hosts;
	}

	private function _request($method, $urls, $data, $callback, &$param, $timeout, $headers)
	{
		assert(!$data || count($urls) == count($data));

		$this->m_curls = array();
		for($i=0; $i < count($urls); $i++){
			$curl = curl_init($urls[$i]);
			$this->_setopt($method, $curl, $data ? $data[$i] : null, $headers);
			curl_multi_add_handle($this->m_multi, $curl);
			$this->m_curls[$i] = $curl;
		}

		if(count($this->m_proxies) > 0){
			foreach($this->m_curls as $i => $curl){
				$j = $i % count($this->m_proxies);
				$proxy = split(":", $this->m_proxies[$j]);
				if(count($proxy) > 1){
					curl_setopt($curl, CURLOPT_PROXYPORT, $proxy[1]);
				} else {
					curl_setopt($curl, CURLOPT_PROXYPORT, 80);
				}
				curl_setopt($curl, CURLOPT_PROXY, $proxy[0]);
				curl_setopt($curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			}
		}

		$r = $this->_perform($this->m_multi, $timeout, $callback, $param);

		foreach($this->m_curls as $i => $curl){
			if(is_callable($callback)) {
				call_user_func($callback, &$param, $i, CURLE_OPERATION_TIMEOUTED, "", "");
			}
			curl_multi_remove_handle($this->m_multi, $curl);
			curl_close($curl);
		}

		return $r;
	}

	private function _perform($multi, $timeout, $callback, &$param)
	{
		// running		
		do{
			$status = curl_multi_exec($multi, $active);
		} while($status === CURLM_CALL_MULTI_PERFORM);

		while(CURLM_OK === $status && $active > 0){
			if(curl_multi_select($multi, $timeout) < 1) {
				// log timeout
				return -1;
			}

			do{
				$status = curl_multi_exec($multi, $active);
			} while($status === CURLM_CALL_MULTI_PERFORM);

			$this->_read($multi, $callback, $param);
		}

		if(CURLM_OK !== $status || 0 !== $active){
			// log
			return -1;
		}
		return 0;
	}

	private function _read($multi, $callback, &$param)
	{
		$info = curl_multi_info_read($multi);
		while($info){
			assert(CURLMSG_DONE === $info["msg"]); // Other return values are currently not available.

			$head = "";
			$body = "";
			$code = $info["result"];
			$curl = $info["handle"];
			if(CURLE_OK === $code){				
				$html = curl_multi_getcontent($curl);
				$size = curl_getinfo($curl, CURLINFO_HEADER_SIZE);
				if($size > 0 && strlen($html) >= $size){
					$head = substr($html, 0, $size);
					$body = substr($html, $size);
				}
			}

			// find curl
			foreach($this->m_curls as $idx => $v){
				if($v == $curl)
					break;
			}

			// callback
			if(is_callable($callback)) {
				call_user_func($callback, &$param, $idx, $code, $head, $body);
			}

			// remove
			unset($this->m_curls[$idx]);
			curl_multi_remove_handle($this->m_multi, $curl);
			curl_close($curl);

			// next
			$info = curl_multi_info_read($multi);
		}
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
	private $m_curls = null;
	private $m_proxies = null;
	private $m_options = null;
	private $m_headers = null;	
};
?>
