<?php
	function zol_wallpaper_image($uri)
	{
		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//ul[@id='showImg']/li/a/img");

		$album = array();

		if (!is_null($elements)) {
			foreach ($elements as $element) {
				$src = $element->getattribute('src');
				if(strlen($src) < 1){
					$src = $element->getattribute('srcs');
				}

				if(strlen($src) > 0){
					// /sjbizhi/images/6/120x90/1390289717341.jpg => /sjbizhi/images/6/240x320/1390289717341.jpg
					$file = basename($src);
					$dir = dirname(dirname($src));
					$album[] = array("dir" => $dir, "file" => $file);
				}
			}
		}

		return $album;
	}

	function zol_wallpaper_album($uri, $sort, $page, $orient)
	{
		if(!strstr(basename($uri), "_p")){
			if(0 == strcmp("seascape", $orient)){
				$uri = substr($uri, 0, -1) . "_p2/";
			}
			else if(0 == strcmp("landscape", $orient)){
				$uri = substr($uri, 0, -1) . "_p3/";
			}
		}

		if(0==strcmp("hot", $sort)){
			$uri = $uri . 'hot_' . $page . '.html';
		} else if($page > 1){
			$uri = $uri . $page . '.html';
		}

		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//li[@class='photo-list-padding']/a");

		$pages = xpath_query($doc, "//div[@class='page']/*/text()");
		$check_page = (0==$pages->length && 1==$page)? 1 : 0;
		foreach ($pages as $p) {
			if(XML_TEXT_NODE == $p->nodeType){
				if((int)$p->wholeText >= $page){
					$check_page = 1;
					break;
				}
			}
		}

		$album = array();
		if (1 == $check_page && !is_null($elements)) {
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getAttribute('href');
				$title = $element->getAttribute('title');
				if(strlen($title) < 1){
					$title = $element->firstChild->getAttribute('title');
				}
				if(strlen($href) > 0){
					$album[$title] = 'http://' . $host["host"] . $href;
				}
			}
		}

		return $album;
	}

	function zol_wallpaper_subcatalog($uri)
	{
		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//dl[@class='filter-item clearfix subcateClass']/dd");

		$catalog = array();
		//$catalog['all'] = $uri;

		if (!is_null($elements)) {
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$nodes = $element->childNodes;
				foreach ($nodes as $node) {
					if(XML_ELEMENT_NODE == $node->nodeType){
						$href = $node->getAttribute('href');
						if(strlen($href) > 0){
							$catalog[$node->nodeValue] = 'http://' . $host["host"] . $href;
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
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//dl[@class='filter-item first clearfix']/dd");

		$catalog = array();
		if (!is_null($elements)) {
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$nodes = $element->childNodes;
				foreach ($nodes as $node) {
					if(XML_ELEMENT_NODE == $node->nodeType){
						$href = $node->getAttribute('href');
						if(strlen($href) > 0){
							//$catalog[$node->nodeValue] = zol_wallpaper_subcatalog('http://sj.zol.com.cn' . $href);
							$catalog[$node->nodeValue] = 'http://' . $host["host"] . $href;
						}
					}
				}
			}
		}

		return $catalog;
	}

	function zol_wallpaper_index($device)
	{
		$devices = array(
			"/iphone5.*/" => 'http://sj.zol.com.cn/bizhi/640x1136/',
			"/iphone4.*/" =>  'http://sj.zol.com.cn/bizhi/640x960/',
			"/iphone3.*/" => 'http://sj.zol.com.cn/bizhi/320x480/',
			"/iphone\d*/" => 'http://sj.zol.com.cn/bizhi/640x1136/', // iphone6/iphone7/iphone8
			"/ipad3.*/" => 'http://desk.zol.com.cn/1024x1024_p3/', //'http://desk.zol.com.cn/2048x2048_p3/',
			"/ipadair.*/" => 'http://desk.zol.com.cn/1536x2048_p3/',
			"/ipadmini2.*/" => 'http://desk.zol.com.cn/1536x2048_p3/',
			"/ipadmini.*/" => 'http://desk.zol.com.cn/1024x1024_p3/',
			"/ipad2.*/" => 'http://sj.zol.com.cn/bizhi/640x960/', //'http://desk.zol.com.cn/1024x1024_p3/',
			"/ipad\D*/" => 'http://sj.zol.com.cn/bizhi/640x960/', //'http://desk.zol.com.cn/1024x1024_p3/',
			"/ipad\d*/" => 'http://desk.zol.com.cn/2048x2048_p3/', // ipad4/ipad5/ipad6/ipad7/ipad8
			"/ipod.*/" => 'http://sj.zol.com.cn/bizhi/640x960/', 
			"I9300" => 'http://sj.zol.com.cn/bizhi/720x1280/',
			"I9100" => 'http://sj.zol.com.cn/bizhi/480x800/',
			"HTCOne" => 'http://sj.zol.com.cn/bizhi/720x1280/',
			"mi2" => 'http://sj.zol.com.cn/bizhi/720x1280/', // xiao mi 2
			"mi1s" => 'http://sj.zol.com.cn/bizhi/480x854/', // xiao mi 1s
			"k860" => 'http://sj.zol.com.cn/bizhi/720x1280/' // lenove K860
		);
		
		//return $devices[$device];
		foreach($devices as $key => $value){
			if(preg_match($key, $device)){
				return $value;
			}
		}
		
		return 'http://sj.zol.com.cn/bizhi/640x960/';
	}

	function zol_wallpaper_resolution($device)
	{
		$uri = zol_wallpaper_index($device);
		$size = basename($uri);
		sscanf($size, "%dx%d", $width, $height);
		return $width . "x" . $height;
	}

	function zol_wallpaper_device($device)
	{
		$mc = new Memcached();
		$mc->addServer("localhost", 11211);

		$mckey = "catalog-" . $device;
		$catalogs = $mc->get($mckey);

		if (!$catalogs) {
			$catalogs = array();
			$uri = zol_wallpaper_index($device);
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
			$catalogs = json_decode($catalogs, True);
		}

		return $catalogs;
	}

	function zol_wallpaper_find($device, $catalog)
	{
		$catalogs = zol_wallpaper_device($device);
		foreach($catalogs as $i){
			if(0 == strcmp($i["name"], $catalog)){
				return $i["uri"];
			}
			
			foreach($i["subcatalog"] as $key => $value){
				if(0 == strcmp($key, $catalog)){
					return $value;
				}
			}
		}
		return ""; // not found
	}
?>
