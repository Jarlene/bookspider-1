<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");

	
	class C77NT
	{
		function GetName()
		{
			return "77nt";
		}
		
		function _77nt_audio($uri)
		{
			$uri = str_replace("Play", "zyurl", $uri);
			$headers = http_get_headers($uri, "Location");

			if(!preg_match("/Location:([^\r\n]*)/i", $headers, $matches)){
				return "";
			}

			return 2 == count($matches) ? iconv("gb2312", "UTF-8", $matches[1]) : "";
		}

		function GetChapters($bookid)
		{
			list($path, $id) = split("-", $bookid);
			$uri = "http://http://www.77nt.com/" . $path . "/List_ID_" . $id . "html";
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$icons = xpath_query($doc, "//div[@class='conlist']/ul/li[1]/img");
			$elements = xpath_query($doc, "//ul[@class='compress']/ul/div/li/span/a");

			$host = parse_url($uri);

			$iconuri = "";
			$summary = "";
			foreach($icons as $icon){
				$href = $icon->getattribute('src');
				$iconuri = 'http://' . $host["host"] . $href;
			}

			$chapters = array();
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$chapter = $element->nodeValue;

				if(strlen($href) > 0 && strlen($chapter) > 0){
					$chapters[] = array("name" => $chapter, "uri" => 'http://' . $host["host"] . $href);
				}
			}

			$data = array();
			$data["icon"] = $iconuri;
			$data["info"] = $summary;
			$data["chapter"] = $chapters;
			return $data;
		}

		function __ParseBooks($response, &$books)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='clist']/ul/li/a");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->getattribute('title');

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".Htm");
					$books[dirname($href) . '-' . substr($bookid, 8)] = $book;
				}
			}
		}
		
		function GetBooks($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$options = xpath_query($doc, "//select[@name='select']/option");
			$elements = xpath_query($doc, "//div[@class='pingshu_ysts8']/ul/li/a");

			$books = array();

			$host = parse_url($uri);
			foreach ($options as $option) {
				$href = $option->getattribute('value');
				if(strlen($href) > 0){
					$u = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $href;
					if(0 != strcmp($u, $uri)){
						$response = http_get($u);
						$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
					}

					$this->__ParseBooks($u, $response, $books);
				}
			}

			$data = array();
			$data["icon"] = "";
			$data["book"] = $books;
			return $data;
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
	//print_r(_77nt_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(_77nt_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(_77nt_chapters('http://www.77nt.com/LiShiPingShu/List_ID_8333.html'));
	//print_r(_77nt_audio('http://www.77nt.com/Play.aspx?id=9184&page=0'));
?>
