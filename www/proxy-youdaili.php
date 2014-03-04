<?php
	require("php/http-multiple.inc");
	require("php/dom.inc");

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
	
	function OnReadData($param, $header, $body, $idx)
	{
		file_put_contents("0_" . $idx . ".html", $body);
		global $proxies;
		$r = Parse($body);
		foreach($r as $proxy){
			$proxies[] = $proxy;
		}
	}

	$urls = array("http://www.youdaili.cn/Daili/guonei/1750.html");
	$dir = dirname($urls[0]);
	$name = basename($urls[0], ".html");
	for($i = 2; $i <= 5; $i++){
		$urls[] = "$dir/$name" . "_$i.html";
	}

	$f = file_get_contents("proxy.cfg");
	$proxies = array();
	$http = new HttpMultiple();
	$http->get($urls, 'OnReadData', $proxies);
	
	$http->setproxy(split(",", $f));
	print_r(count($proxies));
	//file_put_contents("proxy.cfg", implode(",", $proxies));	
?>
