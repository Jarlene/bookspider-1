<?php
function http_get($uri, $timeout=20, $proxy="", $headers=array())
{
	$_headers = array();
	foreach($headers as $header){
		$i = strpos($header, ':');
		$_headers[substr($header, 0, $i)] = substr($header, $i+1);
	}

	$http = new Http();
	$http->settimeout($timeout);
	$http->setproxy($proxy);
	return $http->get($uri, $_headers);
}

function http_post($uri, $data, $timeout=20, $proxy="", $headers=array())
{
	$_headers = array();
	foreach($headers as $header){
		$i = stripos($header, ':');
		$_headers[substr($header, 0, $i)] = substr($header, $i+1);
	}

	$http = new Http();
	$http->settimeout($timeout);
	$http->setproxy($proxy);
	return $http->post($uri, $data, $_headers);
}

function http_get_headers($uri, $timeout=20, $proxy="", $headers=array())
{
	$_headers = array();
	foreach($headers as $header){
		$i = stripos($header, ':');
		$_headers[substr($header, 0, $i)] = substr($header, $i+1);
	}

	$http = new Http();
	$http->settimeout($timeout);
	$http->setproxy($proxy);
	$r = $http->get($uri, $_headers);
	return $http->get_header();
}

// http://blog.csdn.net/skillcoding/article/details/6778015
class Http
{
	// Options: timeout/port/cookie/ssl/proxy/gzip
	// e.g. $options("timeout" => 20, "port" => 80)
	function __construct()
	{
		function_exists('curl_init') || die('CURL Library Not Loaded');     //exit if don't install curl

		$this->m_curl = curl_init();
		$this->_init();
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

	// return response HTTP headers
	function get_header()
	{
		return $this->result["header"];
	}
	
	function get_code()
	{
		return curl_getinfo($this->m_curl, CURLINFO_HTTP_CODE);
	}
	
	function settimeout($timeout)
	{
		curl_setopt($this->m_curl, CURLOPT_CONNECTTIMEOUT, $timeout); // connection timeout
		curl_setopt($this->m_curl, CURLOPT_TIMEOUT, $timeout); // execute timeout
	}

	function setproxy($proxy, $proxyType='HTTP')
	{
		if(strlen($proxy) > 0){
			list($ip, $port) = explode(':', $proxy);

			$proxyType = 0==stricmp($proxyType, 'HTTP') ? CURLPROXY_HTTP : CURLPROXY_SOCKS5;
			curl_setopt($this->m_curl, CURLOPT_PROXYTYPE, $proxyType);
			curl_setopt($this->m_curl, CURLOPT_PROXY, $ip);
			curl_setopt($this->m_curl, CURLOPT_PROXYPORT, $port);

			if(False){
				$proxyAuthType = $options['proxyAuthType']=='BASIC' ? CURLAUTH_BASIC : CURLAUTH_NTLM;  
				$proxyUser = "[{$options['proxyAuthUser']}]:[{$options['proxyAuthPwd']}]";  
				curl_setopt($this->m_curl, CURLOPT_PROXYAUTH, $proxyAuthType);  
				curl_setopt($this->m_curl, CURLOPT_PROXYUSERPWD, $proxyUser);  
			}
		} else {
			curl_setopt($this->m_curl, CURLOPT_PROXY, ""); // clear proxy
		}
	}

	function setssl($ssl)
	{
		// ssl
		if($ssl){
			curl_setopt($this->m_curl, CURLOPT_SSL_VERIFYPEER, false);
			curl_setopt($this->m_curl, CURLOPT_SSL_VERIFYHOST, true);
		}
	}
	
	function setcookie($cookfile)
	{
		curl_setopt($this->m_curl, CURLOPT_COOKIEJAR, $cookfile);
		curl_setopt($this->m_curl, CURLOPT_COOKIEFILE, $cookfile);
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

	private function _init()
	{
		//curl_setopt($this->m_curl, CURLOPT_PORT, 80); // HTTP default port
		curl_setopt($this->m_curl, CURLOPT_FOLLOWLOCATION, true); // enable HTTP Location header
		curl_setopt($this->m_curl, CURLOPT_AUTOREFERER, true);
		curl_setopt($this->m_curl, CURLOPT_MAXREDIRS, 8); // enable HTTP Location header
		//curl_setopt($this->m_curl, CURLOPT_REFERER, $options['Referer']);
		//curl_setopt($this->m_curl, CURLOPT_USERAGENT, $options['User-Agent']);  // default user-agent
		curl_setopt($this->m_curl, CURLOPT_HEADER, true); // get HTTP header
		curl_setopt($this->m_curl, CURLOPT_RETURNTRANSFER, true); // get HTTP body
		curl_setopt($this->m_curl, CURLOPT_BINARYTRANSFER, true); // binary content
		curl_setopt($this->m_curl,CURLOPT_ENCODING, ''); // decode gzip
	}

	private function _request($method, $uri, $data, $headers)
	{
		curl_setopt($this->m_curl, CURLOPT_URL, $uri);

		$headers = $this->_headers($headers);		
		curl_setopt($this->m_curl, CURLOPT_HTTPHEADER, $headers);

		if($method == "POST"){
			curl_setopt($this->m_curl, CURLOPT_POST, true);
			curl_setopt($this->m_curl, CURLOPT_POSTFIELDS, $data);	
		}

		// grab URL and pass it to the browser
		//$t = gettimeofday(true);
		$response = curl_exec($this->m_curl);
		//file_put_contents ("qiubai.html", $response);

		$code = curl_getinfo($this->m_curl, CURLINFO_HTTP_CODE);
		if(200 == $code){
			$headerSize = curl_getinfo($this->m_curl, CURLINFO_HEADER_SIZE);
			$this->result['header'] = substr($response, 0, $headerSize);
			$this->result['body'] = substr($response, $headerSize);

			if(0 == $this->errno())
				return $this->result['body'];
		}
		return "";
	}

	private function _headers($headers)
	{
		$headers0 = array(
			'Accept' => 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
			'Accept-Encoding' => 'gzip, deflate',
			'Accept-Language' => 'en-US,en;q=0.5',
			'User-Agent' => 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:31.0) Gecko/20100101 Firefox/31.0',
		);

		$headers = array_merge($headers0, $headers);

		$result = array();
		foreach($headers as $name => $value){
			$result[] = "$name: $value";
		}

		return $result;
	}

	private $m_curl = null;
	private $m_result = null;
};

// usage:
// echo http_get("http://www.baidu.com/");
// echo http_post("http://www.17tsw.com/SoClass.aspx", "class=%C8%F6%BB%D1&submit=&ctl00%24Sodaohang=", array("Referer: http://www.17tsw.com/"));
// http_get("http://www.baidu.com/", "15", "183.224.1.55:80", array('referer' => 'http://www.baidu.com/', 'User-Agent' => 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0'));
?>
