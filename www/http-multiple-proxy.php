<?php
class HttpMultipleProxy
{
	function __construct($file)
	{
		$this->m_file = $file;
		
		$content = file_get_contents($file);
		$this->m_proxies = split(",", $content);
	}
	
	function __destruct()
	{
		file_put_contents($this->m_file, implode(",", $this->m_proxies));
	}
	
	function get($urls, $callback, &$param, $timeout=20, $headers=array())
	{
	}
	
	function post($urls, $callback, &$param, $data, $timeout=20, $headers=array())
	{
	}
	
	private function _request($method, $urls, $data, $proxies)
	{
		srand(time());
		$http = new HttpMultiple();

		$n = rand();
		$proxies = array();
		for($i = 0; $i < count($urls) && $i < count($this->m_proxies); $i++){
			$j = ($i + $n) % count($this->m_proxies);
			$proxies[] = $this->m_proxies[$j];
		}

		$result = array();
		$http->setproxy($proxies);
		$r = $http->get($urls, array($this, '_on_request'), $result);

		// tidy proxy list
		$this->m_proxies = array();
		foreach($proxies as $proxy){
			$this->m_proxies[] = $proxy;
		}

		$rTask = count($urls);
		$rProxy = count($this->m_proxies);
		echo "Task $rTask/$nTask, Proxy: $rProxy/$nProxy \r\n";

		return $r;
	}
	
	private function _on_request(&$p, $i, $r, $header, $body)
	{
		if(0 != $r){
			unset($p[$i]);
		} else {
			$p[$i] = $body;
		}
	}
};
?>
