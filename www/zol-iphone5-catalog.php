<?php
	require("php/http.inc");
	require("php/dom.inc");

	function zol_wallpaper_subcatalog($uri)
	{
		$response = http_get($uri);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//dl[@class='filter-item clearfix subcateClass']/dd");

		$catalog = array();
		$catalog['all'] = $uri;

		if (!is_null($elements)) {
			foreach ($elements as $element) {
				$nodes = $element->childNodes;
				foreach ($nodes as $node) {
					if(XML_ELEMENT_NODE == $node->nodeType){
						$href = $node->getAttribute('href');
						if(strlen($href) > 0){
							$catalog[$node->nodeValue] = $href;
						}
					}
				}
			}
		}
		
		return $catalog;
	}

	$uri = 'http://sj.zol.com.cn/bizhi/640x1136/';
	$response = http_get($uri);

	$doc = dom_parse($response);
	$elements = xpath_query($doc, "//dl[@class='filter-item first clearfix']/dd");

	$catalog = array();
	if (!is_null($elements)) {
		foreach ($elements as $element) {
			$nodes = $element->childNodes;
			foreach ($nodes as $node) {
				if(XML_ELEMENT_NODE == $node->nodeType){
					$href = $node->getAttribute('href');
					if(strlen($href) > 0){
						$catalog[$node->nodeValue] = zol_wallpaper_subcatalog('http://sj.zol.com.cn' . $href);
					}
				}
			}
		}
	}
	
	print_r($catalog);
?>
