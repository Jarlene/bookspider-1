<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");

	function ysts8_audio($uri)
	{
		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		
		if(!preg_match('/\"\/play\/flv\.html\?(.+?)\"/', $response, $matches)){
			return "";
		}

		return 2 == count($matches) ? iconv("gb2312", "UTF-8", $matches[1]) : "";
	}

	function ysts8_chapters($uri)
	{
		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//div[@class='ny_l']/ul/li/a");

		$chapters = array();

		if (!is_null($elements)) {
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$chapter = $element->nodeValue;

				if(strlen($href) > 0 && strlen($chapter) > 0){
					$chapters[$chapter] = 'http://' . $host["host"] . $href;
				}
			}
		}

		return $chapters;
	}

	function ysts8_works($uri)
	{
		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//div[@class='pingshu_ysts8']/ul/li/a");

		$books = array();

		if (!is_null($elements)) {
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->nodeValue;

				if(strlen($href) > 0 && strlen($book) > 0){
					$books[$book] = 'http://' . $host["host"] . $href;
				}
			}
		}

		return $books;
	}

	function ysts8_artist($uri)
	{
		$uri = 'http://www.pingshu8.com/Music/bzmtv_1.Htm';
		$response = http_get($uri);
		$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
		$doc = dom_parse($response);
		$elements = xpath_query($doc, "//div[@class='t2']/ul/li/a");

		$artists = array();

		if (!is_null($elements)) {
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$artist = $element->nodeValue;

				//$artist = mb_convert_encoding($artist, "gb2312", "UTF-8");
				//$artist = mb_convert_encoding($artist, "UTF-8", "gb2312");
				//$artist = iconv("GB18030", "UTF-8", $artist);
				if(strlen($href) > 0 && strlen($artist) > 0){
					$artists[$artist] = 'http://' . $host["host"] . $href;
				}
			}
		}

		return $artists;
	}

	function ysts8_all()
	{
		$all = array();
		$artists = pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm');
		foreach($artists as $artist => $href){
			$all[$artist] = $href;
		}
		
		$artists = pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm');
		foreach($artists as $artist => $href){
			$all[$artist] = $href;
		}
		return $all;
	}
	
	function ysts8_api_artist()
	{
		$mc = new Memcached();
		$mc->addServer("localhost", 11211);
		
		$mckey = "artists";
		$artists = $mc->get($mckey);

		if(!$artists){
			$artists = pingshu8_all();
			$mc->set($mckey, json_encode($artists), 23*60*60);
		} else {
			$artists = json_decode($artists, True);
		}
		
		return $artists;
	}
	
	function ysts8_api_works($artist)
	{
		$mc = new Memcached();
		$mc->addServer("localhost", 11211);

		$mckey = "artists-" . $artist;
		$artists = $mc->get($mckey);

		if(!$artists){
			$uri = "";
			$artists = pingshu8_api_artist();
			foreach($artists as $key => $href){
				if(0==strcmp($key, $artist)){
					$uri = $href;
					break;
				}
			}

			if(strlen($uri) < 1)
				return array();

			$works = pingshu8_works($uri);
			$mc->set($mckey, json_encode($works), 23*60*60);
		} else {
			$works = json_decode($artists, True);
		}

		return $works;
	}

	// print_r($all);
	//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm'));
	//print_r(ysts8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
	//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
