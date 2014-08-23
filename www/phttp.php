<?php
require_once("php/http.inc");

class PHttp
{
	function get_http()
	{
		return $this->http;
	}

	function get($uri, $pattern, $headers=array())
	{
		if(count($this->proxies) < 0){
			return $this->http->get($uri, $headers);
		} else {
			for($i = 0; $i < 5 && $i < count($this->proxies); $i++){
				$proxy = $this->proxies[$this->offset];
				$this->http->setproxy($proxy);

				$html = $this->http->get($uri, $headers);
				if(strpos($html, $pattern)){
					return $html;
				} else {
					unset($this->proxies[$this->offset]);
				}

				if(count($this->proxies) > 0){
					$this->offset = ($this->offset + 1) % count($this->proxies);
				}
			}
		}

		return "";
	}

	//----------------------------------------------------------------------------
	// constructor
	//----------------------------------------------------------------------------
	function __construct($proxy="proxy.cfg")
	{
		$this->http = new Http();
		$this->http->settimeout(120);

		if(is_file($proxy)){
			$this->proxies = split(",", file_get_contents($proxy));
		} else {
			$this->proxies = array();
		}

		if(count($this->proxies) > 0)
			$this->offset = rand() % count($this->proxies);
	}

	private $http;
	private $proxies;
	private $offset;
}
?>
