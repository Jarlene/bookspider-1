<?php
	require("php/dom.inc");
	require("php/http-multiple.inc");

	$urls = array("http://www.youdaili.cn/Daili/guonei/1783.html");
	$dir = dirname($urls[0]);
	$name = basename($urls[0], ".html");
	for($i = 2; $i <= 5; $i++){
		$urls[] = "$dir/$name" . "_$i.html";
	}

	$proxies = array();
	$http = new HttpMultiple();
	$http->get($urls, 'OnReadData', &$proxies);
	// print_r(count($proxies) . "\r\n");
	 $today = date('Y-m-d');
	// file_put_contents("proxy-$today.cfg", implode(",", $proxies));

	//$f = file_get_contents("proxy-$today.cfg");
	//$proxies = split(",", $f);
	print_r(count($proxies) . "\r\n");
	$proxies = CheckProxy($proxies);
	print_r(count($proxies) . "\r\n");
	file_put_contents("proxy-$today-checked.cfg", implode(",", $proxies));

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
	
	function OnReadData($proxies, $idx, $r, $header, $body)
	{
		//file_put_contents("0_" . $idx . ".html", $body);
		//global $proxies;
		print_r("OnReadData $idx : $r\n");
		
		$r = Parse($body);
		foreach($r as $proxy){
			$proxies[] = $proxy;
		}
	}
	
	function CheckProxy($proxies)
	{
		$rs = array();
		for($i = 0; $i < count($proxies)/100; $i += 1){
			$subproxies = array();
			for($j = $i * 100; $j < ($i+1) * 100 && $j < count($proxies); $j += 1){
				$subproxies[] = $proxies[$j];
			}

			$result = __CheckNProxy($subproxies, 5);
			foreach($result as $proxy){
				$rs[] = $proxy;
			}
		}
		return $rs;
	}

	function __CheckNProxy($proxies, $timeout)
	{
		$t0 = gettimeofday(true);
	
		$urls = array();
		for($i = 0; $i < count($proxies); $i++){
			$urls[] = "http://www.pingshu8.com/music/newzj.htm";
		}
		
		$result = array();
		$http = new HttpMultiple();
		$http->setproxy($proxies);
		$http->get($urls, '__OnCheckProxy', &$result, $timeout);

		$r = array();
		foreach($result as $j){
			$r[] = $proxies[$j];
		}
		
		$t1 = gettimeofday(true);
		echo "time: " . ($t1-$t0) . "\r\n";
		print_r("result: " . count($r) . "\r\n");
		return $r;
	}

	function __OnCheckProxy($result, $idx, $r, $header, $body)
	{
		$n = strpos($body, "luckyzz@163.com");
		if($n){
			$result[] = $idx;
		}
		return 0;
	}
?>
