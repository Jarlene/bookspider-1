<?php
	function zol_wallpaper_image($uri)
	{
		$response = http_get($uri);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//ul[@id='showImg']/li/a/img");

		$album = array();

		if (!is_null($elements)) {
			foreach ($elements as $element) {
				$src = $element->getAttribute('src');
				if(strlen($src) < 1){
					$src = $element->getAttribute('srcs');
				}

				if(strlen($src) > 0){
					$album[] = $src;
				}
			}
		}
		
		return $album;
	}

	function zol_wallpaper_album($uri)
	{
		$response = http_get($uri);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//li[@class='photo-list-padding']/a");

		$album = array();

		if (!is_null($elements)) {
			foreach ($elements as $element) {
				$href = $element->getAttribute('href');
				$title = $element->getAttribute('title');
				if(strlen($href) > 0){
					$album[$title] = 'http://sj.zol.com.cn' . $href;
				}
			}
		}

		return $album;
	}
	
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
	
	function zol_wallpaper_device($device)
	{
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

		$mc = new Memcached();
		$mc->addServer("localhost", 11211);

		$mckey = "catalog-" . $device;
		$catalogs = $mc->get($mckey);

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

				$mc->set($mckey, json_encode($catalogs), 23*60*60);
			}
		} else {
			$catalogs = json_decode($catalogs);
		}

		return $catalogs;
	}
	
	function zol_wallpaper_find($device, $catalog)
	{
		$catalogs = zol_wallpaper_device($device);
		foreach($catalogs as $i){
			if(0 == strcmp($i->name, $catalog)){
				return $i->uri;
			}
			
			foreach($i->subcatalog as $key => $value){
				if(0 == strcmp($key, $catalog)){
					return $value;
				}
			}
		}
		return ""; // not found
	}
?>
