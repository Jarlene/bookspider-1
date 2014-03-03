<?php
	class CPingShu8 
	{
		public $cache = array(
						"catalog" => 86400, // 24*60*60
						"book" => 86400,
						"chapter" => 86400,
						"audio" => 0,
						"search" => 86400
					);

		public $redirect = 0;

		function HttpGet($uri)
		{
			$base = time();
			$proxies = array("113.57.230.83", 
				"114.80.120.53:8080", 
				"117.40.160.45:3128", 
				"117.59.224.62:80", 
				"118.126.5.149:9001", 
				"115.238.191.146:3128", 
				"218.207.83.141:3128", 
				"219.216.110.96:3259",
				"221.130.17.39:80");

			for($i = 0; $i < count($proxies); $i++){
				$proxy = $proxies[($base + $i) % count($proxies)];
				//print_r("proxy: " . $proxy . "\r\n");
				$r = http_get($uri, 5, $proxy);
				if(!$r){
					continue;
				}
				
				if(false !== stripos($r, "pingshu8")){
					return $r;
				}
			}
			
			return http_get($uri, 10, "");
		}

		function GetName()
		{
			return "pingshu8";
		}

		function GetAudio($uri)
		{
			$response = $this->HttpGet($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);

			if(!preg_match('/encodeURI\(\"(.+)\"\)/', $response, $matches)){
				return "";
			}

			if(2 != count($matches)){
				return "";
			}

			$uri = $matches[1];
			$uri = str_replace("@123abc", "9", $uri);
			$uri = str_replace(".flv", ".mp3", $uri);
			$uri = str_replace("play0.", "pl0.", $uri);
			$uri = str_replace("play1.", "pl1.", $uri);

			return iconv("gb18030", "UTF-8", $uri);
		}

		function __ParseChapters($uri, $response)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//li[@class='a1']/a");

			$chapters = array();

			if (!is_null($elements)) {
				$host = parse_url($uri);
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$chapter = $element->nodeValue;

					if(strlen($href) > 0 && strlen($chapter) > 0){
						$chapters[] = array("name" => $chapter, "uri" => 'http://' . $host["host"] . $href);
					}
				}
			}

			return $chapters;
		}
		
		function GetChapters($bookid)
		{
			$uri = "http://www.pingshu8.com/MusicList/mmc_" . $bookid . ".htm";
			$response = $this->HttpGet($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$icons = xpath_query($doc, "//div[@class='a']/img");
			$infos = xpath_query($doc, "//div[@class='c']/div");
			$options = xpath_query($doc, "//select[@name='turnPage']/option");

			$host = parse_url($uri);
			$iconuri = "";
			$summary = "";
			foreach($icons as $icon){
				$href = $icon->getattribute('src');
				if(0==strncmp("../", $href, 3)){
					$iconuri = 'http://' . $host["host"] . dirname(dirname($host["path"])) . '/' . substr($href, 3);
				} else {
					$iconuri = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $href;
				}
			}
			foreach($infos as $info){
				$summary = $info->nodeValue;
			}

			$chapters = array();

			if (!is_null($options)) {
				foreach ($options as $option) {
					$href = $option->getattribute('value');
					if(strlen($href) > 0){
						$u = 'http://' . $host["host"] . $href;
						if(0 != strcmp($u, $uri)){
							$response = $this->HttpGet($u);
							$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
						}

						$nchapters = $this->__ParseChapters($u, $response);
						$chapters = array_merge($chapters, $nchapters);
					}
				}
			} else {
				$nchapters = $this->__ParseChapters($uri, $response);
				$chapters = array_merge($chapters, $nchapters);
			}

			$data = array();
			$data["icon"] = $iconuri;
			$data["info"] = $summary;
			$data["chapter"] = $chapters;
			return $data;
		}

		function GetBooks($uri)
		{
			$books = array();
			$iconuri = "";

			$response = $this->HttpGet($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			
			if(0 == strcmp($uri, 'http://www.pingshu8.com/music/newzj.htm')){
				$elements = xpath_query($doc, "//div[@class='tab3']/ul/li[2]/a[2]");
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$n = strpos($bookid, '.');
						$books[substr($bookid, 4, $n-4)] = $book;
					}
				}
			} else if(0 == strcmp($uri, 'http://www.pingshu8.com/top/pingshu.htm')){
				$elements = xpath_query($doc, "//div[@class='tab3']/a");
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$n = strpos($bookid, '.');
						$books[substr($bookid, 4, $n-4)] = $book;
					}
				}
			} else {
				$icons = xpath_query($doc, "//div[@class='z4']/img");
				$elements = xpath_query($doc, "//div[@class='jj2']/div/div/a");

				$host = parse_url($uri);

				foreach($icons as $icon){
					$href = $icon->getattribute('src');
					if(0==strncmp("../", $href, 3)){
						$iconuri = 'http://' . $host["host"] . dirname(dirname($host["path"])) . '/' . substr($href, 3);
					} else {
						$iconuri = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $href;
					}
				}

				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$n = strpos($bookid, '.');
						$books[substr($bookid, 4, $n-4)] = $book;
					}
				}
			}

			$data = array();
			$data["icon"] = $iconuri;
			$data["book"] = $books;
			return $data;
		}

		function __GetSubcatalog($uri)
		{
			$response = $this->HttpGet($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);

			$xpath = new XPath($response);
			$elements = $xpath->query("//div[@class='t2']/ul/li/a");

			$artists = array();
			$artists["最近更新"] = 'http://www.pingshu8.com/music/newzj.htm';
			$artists["排行榜"] = 'http://www.pingshu8.com/top/pingshu.htm';

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

			return $artists;
		}

		function GetCatalog()
		{
			$catalog = array();
			$catalog["评书"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_1.Htm');
			$catalog["相声小品"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_2.Htm');
			$catalog["小说"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_3.Htm');
			$catalog["金庸全集"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_4.Htm');
			$catalog["综艺娱乐"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_5.Htm');
			return $catalog;
		}

		function __SearchAuthor($keyword)
		{
			$uri = "http://www.pingshu8.com/bzmtv_inc/SingerSearch.asp?keyword="  . urlencode(iconv("UTF-8", "gb2312", $keyword));
			$response = $this->HttpGet($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//table[@class='TableLine']/form/tr/td[1]/div/a");

			$artists = array();

			if (!is_null($elements)) {
				foreach ($elements as $element) {
					$artist = $element->nodeValue;
					if(strlen($artist) > 0){
						$artists[] = $artist;
					}
				}
			}

			return $artists;
		}

		function __SearchBook($keyword)
		{
			$uri = "http://www.pingshu8.com/bzmtv_inc/SpecialSearch.asp?keyword=" . urlencode(iconv("UTF-8", "gb2312", $keyword));
			$response = $this->HttpGet($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$xpath = new DOMXpath($doc);
			$elements = $xpath->query("//table[@class='TableLine']/tr/td[1]/div/a");

			$books = array();
			if (!is_null($elements)) {
				$host = parse_url($uri);
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					$bookid = basename($href);
					$n = strpos($bookid, '.');
					$books[substr($bookid, 4, $n-4)] = $book;
				}
			}
			return $books;
		}

		function Search($keyword)
		{
			$authors = $this->__SearchAuthor($keyword);
			$books = $this->__SearchBook($keyword);
			return array("catalog" => $authors, "book" => $books);
		}
	}

	//$pinshu8 = new CPingShu8();
	//print_r($pinshu8->__SearchBook("王"));
	//print_r($pinshu8->__SearchAuthor("王"));
	// print_r($all);
	//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm'));
	//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(pingshu8_chapters('http://www.pingshu8.com/MusicList/mmc_99_255_1.Htm'));
	//print_r(pingshu8_audio('http://www.pingshu8.com/play_24034.html'));
?>
