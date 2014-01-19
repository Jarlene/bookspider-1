<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");

	function zol_wallpaper_subcatalog($uri)
	{
		$response = http_get($uri);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//dl[@class='filter-item clearfix subcateClass']/dd");

		$catalog = array();
		//$catalog['all'] = $uri;

		if (!is_null($elements)) {
			foreach ($elements as $element) {
				$nodes = $element->childNodes;
				foreach ($nodes as $node) {
					if(XML_ELEMENT_NODE == $node->nodeType){
						$href = $node->getAttribute('href');
						if(strlen($href) > 0){
							$catalog[$node->nodeValue] = 'http://sj.zol.com.cn' . $href;
						}
					}
				}
			}
		}
		
		return $catalog;
	}

	function zol_wallpaper_catalog($uri)
	{
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
							//$catalog[$node->nodeValue] = zol_wallpaper_subcatalog('http://sj.zol.com.cn' . $href);
							$catalog[$node->nodeValue] = 'http://sj.zol.com.cn' . $href;
						}
					}
				}
			}
		}

		return $catalog;
	}

	$devices = array(
		"iphone5" => 'http://sj.zol.com.cn/bizhi/640x1136/',
		"iphone4" => 'http://sj.zol.com.cn/bizhi/640x960/',
		"iphone4s" =>  'http://sj.zol.com.cn/bizhi/640x960/',
		"I9300" => 'http://sj.zol.com.cn/bizhi/720x1280/',
		"I9100" => 'http://sj.zol.com.cn/bizhi/480x800/',
		"HTCOne" => 'http://sj.zol.com.cn/bizhi/720x1280/',
		"mi2" => 'http://sj.zol.com.cn/bizhi/720x1280/', // xiao mi 2
		"mi1s" => 'http://sj.zol.com.cn/bizhi/480x854/', // xiao mi 1s
		"k860" => 'http://sj.zol.com.cn/bizhi/720x1280/' // lenove K860
	);

	$page = php_reqvar("page", "1");
	$device = php_reqvar("device", 'iphone5');
	$limit = php_reqvar("limit", 50);

	$mc = new Memcached();
	$mc->addServer("localhost", 11211);

	$catalogKey = "catalog-" . $device . "-" . $page;
	$catalogs = $mc->get($catalogKey);

	if (!$catalogs) {
		$catalogs = array();
		$uri = $devices[$device];
		if($uri){
			$catalog["name"] = "all";
			$catalog["uri"] = $uri;
			$catalog["subcatalog"] = array();
			$catalogs[] = $catalog;

			$subcatalog = zol_wallpaper_catalog($uri);
			foreach($subcatalog as $key => $value){
				$catalog["name"] = $key;
				$catalog["uri"] = $value;
				$catalog["subcatalog"] = zol_wallpaper_subcatalog($value);
				$catalogs[] = $catalog;
			}

			$mc->set($catalogKey, json_encode($catalogs), 20*60*60);
		}
	} else {
		$catalogs = json_decode($catalogs);
	}
	
	$data = array();
	foreach($catalogs as $i){
		$subcatalog = array();
		foreach($i->subcatalog as $key => $value){
			$subcatalog[] = $key;
		}
		
		$catalog = array();
		$catalog["catalog"] = $i->name;
		$catalog["subcatalog"] = $subcatalog;
		$data[] = $catalog;
	}
	
	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);
?>
