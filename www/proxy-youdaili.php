<?php
	require("php/dom.inc");
	require("php/http-multiple.inc");	
	require("http-multiple-proxy.inc");
	
	function Parse($response)
	{
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//div[@class='cont_font']/p");

		$proxies = array();
		foreach ($elements as $element) {
			foreach($element->childNodes as $node){
				if(XML_TEXT_NODE == $node->nodeType){
					$value = $node->nodeValue;
					$n = strpos($value, "@");
					if($n > 0)
						$proxies[] = trim(substr($value, 0, $n));
				}
			}
		}

		return $proxies;
	}
	
	function OnReadData(&$proxies, $header, $body, $idx)
	{
		//file_put_contents("0_" . $idx . ".html", $body);
		//global $proxies;
		$r = Parse($body);
		foreach($r as $proxy){
			$proxies[] = $proxy;
		}
	}

	function OnCheckProxy(&$param, $header, $body, $idx)
	{
		if(strpos($body, "ProxyTestWebPage")){
		}
	}

	function CheckProxy($proxies)
	{
		$urls = array();
		for($i = 0; $i < 100; i++){
			$urls[] = "http://115.28.54.237/joke/proxy.html";
		}

		$result = array();
		$http = new HttpMultiple();
		for($i = 0; $i < count($proxies); $i += 100){
			$subproxies = array();
			for($j = 0; $j < 100; j++){
			$subproxies = $proxies[i];			
			$http->setproxy($proxies);
			$http->get($urls, 'OnCheckProxy', $result);
		}
	}

	$urls = array("http://www.youdaili.cn/Daili/guonei/1755.html");
	$dir = dirname($urls[0]);
	$name = basename($urls[0], ".html");
	for($i = 2; $i <= 5; $i++){
		$urls[] = "$dir/$name" . "_$i.html";
	}
	
	$proxies = array();
	$http = new HttpMultiple();
	$http->get($urls, 'OnReadData', $proxies);
	//print_r(count($proxies));	
	$today = date('Y-m-d');
	file_put_contents("proxy-$doday.cfg", implode(",", $proxies));

	//$f = file_get_contents("proxy-$doday.cfg");
	//CheckProxy(split(",", $f));
?>
