<?php
function http_options($timeout, $proxy)
{
	$options = array('timeout' => $timeout);
	if(strlen($proxy) > 0){
		$items = split(":", $proxy);
		$options["proxy"] = true;
		$options["proxyType"] = CURLPROXY_HTTP;
		$options["proxyHost"] = $items[0];
		if(count($items) > 1){
			$options["proxyPort"] = $items[1];
		} else {
			$options["proxyPort"] = 80;
		}
	}

	return $options;
}

function http_get($uri, $timeout=20, $proxy="", $headers=array())
{
	$options = http_options($timeout, $proxy);
	$http = new Http($options);
	$r = $http->get($uri, $headers);
	return $r ? $r['body'] : "";
}

function http_post($uri, $data, $timeout=20, $proxy="", $headers=array())
{
	$options = http_options($timeout, $proxy);
	$http = new Http($options);
	$r = $http->post($uri, $data, $headers);
	return $r ? $r['body'] : "";
}

function http_get_headers($uri, $timeout=20, $proxy="", $headers=array())
{
	$options = http_options($timeout, $proxy);
	$http = new Http($options);
	$r = $http->get($uri, $headers);
	return $r ? $r['header'] : "";
}

// http://blog.csdn.net/skillcoding/article/details/6778015
class Http
{
	// Options: timeout/port/cookie/ssl/proxy/gzip
	// e.g. $options("timeout" => 20, "port" => 80)
	function __construct($options=array())
	{
		$this->m_curl = curl_init();
		$this->_init($options);
	}
	
	function __destruct()
	{
		curl_close($this->m_curl);
	}
	
	// headers: referer/cookie/user-agent
	// e.g. get("http://www.baidu.com/", array("Referer" => "http://www.baidu.com"));
	function get($uri, $headers=array())
	{
		return $this->_request("GET", $uri, null, $headers);
	}
	
	// headers: referer/cookie/user-agent
	// e.g. post("http://www.baidu.com/", "query=abc", array("Referer" => "http://www.baidu.com"));
	function post($uri, $data, $headers=array())
	{
		return $this->_request("POST", $uri, $data, $headers);
	}
	
	// string error information
	function error()
	{  
        return curl_error($this->m_curl);  
    }
	
	function errno()
	{
		return curl_errno($this->m_curl);
	}
	
	private function _init($options)
	{
		$options0 = array(
			'port' => 80,
			'timeout' => 20,
			'cookie' => false,
			'ssl' => false,
			'gzip' => true,
			'proxy' => false,
			'proxyAuth' => false);

		$options = array_merge($options0, $options);

		function_exists('curl_init') || die('CURL Library Not Loaded');     //exit if don't install curl

		curl_setopt($this->m_curl, CURLOPT_PORT, $options['port']); // HTTP default port
		curl_setopt($this->m_curl, CURLOPT_FOLLOWLOCATION, true); // enable HTTP Location header
		//curl_setopt($this->m_curl, CURLOPT_USERAGENT, $this->m_headers['User-Agent']);  // default user-agent
		curl_setopt($this->m_curl, CURLOPT_CONNECTTIMEOUT, $options['timeout']); // connection timeout
		curl_setopt($this->m_curl, CURLOPT_TIMEOUT, $options['timeout']); // execute timeout
		curl_setopt($this->m_curl, CURLOPT_HEADER, true); // get HTTP header
		curl_setopt($this->m_curl, CURLOPT_RETURNTRANSFER, true); // get HTTP body
		curl_setopt($this->m_curl, CURLOPT_BINARYTRANSFER, true); // binary content
		curl_setopt($this->m_curl,CURLOPT_ENCODING, ''); // decode gzip
		
		// proxy
		if($options['proxy']){
			$proxyType = $options['proxyType']=='HTTP' ? CURLPROXY_HTTP : CURLPROXY_SOCKS5;
			curl_setopt($this->m_curl, CURLOPT_PROXYTYPE, $proxyType);
			curl_setopt($this->m_curl, CURLOPT_PROXY, $options['proxyHost']);
			curl_setopt($this->m_curl, CURLOPT_PROXYPORT, $options['proxyPort']);
			
			if($options['proxyAuth']){
				$proxyAuthType = $options['proxyAuthType']=='BASIC' ? CURLAUTH_BASIC : CURLAUTH_NTLM;  
                $proxyUser = "[{$options['proxyAuthUser']}]:[{$options['proxyAuthPwd']}]";  
				curl_setopt($this->m_curl, CURLOPT_PROXYAUTH, $proxyAuthType);  
                curl_setopt($this->m_curl, CURLOPT_PROXYUSERPWD, $proxyUser);  
			}
		}
		
		// ssl
		if($options['ssl']){
			curl_setopt($this->m_curl, CURLOPT_SSL_VERIFYPEER, false);
			curl_setopt($this->m_curl, CURLOPT_SSL_VERIFYHOST, true);
		}
		
		// cookie
		if($options['cookie']){
			$cookfile = tempnam(sys_get_temp_dir(), 'curl-cookie-');
			curl_setopt($this->m_curl, CURLOPT_COOKIEJAR, $cookfile);
            curl_setopt($this->m_curl, CURLOPT_COOKIEFILE, $cookfile);
		}
	}
	
	private function _request($method, $uri, $data, $headers)
	{
		$t = gettimeofday(true);
		$headers0 = array( // http headers
			"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/$t",
			'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
			'Accept-Encoding: gzip, deflate',
			'Accept-Language: en-US,en;q=0.5'
		);
		foreach($headers0 as $header){
			$headers[] = $header;
		}

		curl_setopt($this->m_curl, CURLOPT_URL, $uri);
		curl_setopt($this->m_curl, CURLOPT_HTTPHEADER, $headers);

		if($method == "POST"){
			curl_setopt($this->m_curl, CURLOPT_POST, true);
			curl_setopt($this->m_curl, CURLOPT_POSTFIELDS, $data);	
		}

		// if(array_key_exists("Referer", $headers){  
            // curl_setopt($this->m_curl, CURLOPT_REFERER, $headers['Referer']);  
        // }else{  
            // curl_setopt($this->m_curl, CURLOPT_AUTOREFERER, true);  
        // }

		// grab URL and pass it to the browser
		$response = curl_exec($this->m_curl);
		//file_put_contents ("qiubai.html", $response);

		$headerSize = curl_getinfo($this->m_curl, CURLINFO_HEADER_SIZE);
		$result['header'] = substr($response, 0, $headerSize);
		$result['body'] = substr($response, $headerSize);

		if(0 == $this->errno())
			return $result;
		return false;
	}

	private $m_curl = null;
};

// usage:
// echo http_get("http://www.baidu.com/");
// echo http_post("http://www.17tsw.com/SoClass.aspx", "class=%C8%F6%BB%D1&submit=&ctl00%24Sodaohang=", array("Referer: http://www.17tsw.com/"));
?>
