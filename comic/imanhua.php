<?php
	class CIManHua 
	{
		public $cache = array(
						"catalog" => 86400, // 24*60*60
						"book" => 86400,
						"chapter" => 86400,
						"audio" => 600,
						"search" => 86400
					);

		public $redirect = 0;

		function GetName()
		{
			return "imanhua";
		}

		function GetPic($uri)
		{
			$response = http_get($uri);
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
			$response = http_get($uri);
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
							$response = http_get($u);
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

		function __ParseBooks($uri, $response, &$books, $xpath)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, $xpath);

			if (!is_null($elements)) {
				$host = parse_url($uri);
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->firstChild->wholeText;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$n = strpos($bookid, '.');
						$books[substr($bookid, 2, $n-2)] = $book;
					}
				}
			}
		}

		function GetBooks($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$xpath = new XPath($response);
			
			$books = array();
			if(0 == strcmp($uri, 'http://www.ysts8.com/index_tim.html')){
				$this->__ParseBooks($u, $response, $books, "//div[@class='pingshu_ysts8_i']/ul/li/a");
			} else if(0 == strcmp($uri, 'http://www.ysts8.com/index_hot.html')){
				$this->__ParseBooks($u, $response, $books, "//div[@class='pingshu_ysts8_i']/ul/li/a");
			} else {
				$page = $xpath->get_value("//div[@class='pagerHead']/strong[3]", null, 1);

				$host = parse_url($uri);
				for($i = 1; $i <= $page; $i++) {
					if(1 != $i){
						$u = $uri . 'index_p' . $i . '.html';
						$response = http_get($u);
						$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
						$xpath = new XPath($response);
					}
					
					$elements = $xpath->query("//div[@class='main']/div/ul/li");
					foreach ($elements as $element) {
						$book = $xpath->get_attribute("a", "title", $element, "");
						$href = $xpath->get_attribute("a", "href", $element, "");
						$icon = $xpath->get_attribute("a/img", "src", $element, "");
						$time = $xpath->get_attribute("em/a", "title", $element, "");
						$status = $xpath->get_value("em/a", $element, "");

						if(strlen($href) > 0 && strlen($book) > 0){
							$bookid = basename($href);
							$books[$bookid] = array("book" => $book, "uri" => $href, "icon" => $icon, "time" => $time, "status" => $status);
						}
					}
				}
			}
			
			$data = array();
			$data["icon"] = "";
			$data["book"] = $books;
			return $data;
		}

		function GetCatalog()
		{
			$uri = 'http://www.imanhua.com/';
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//ul[@class='navList']/li/a");

			$subcatalogs = array();
			$subcatalogs["最近更新"] = 'http://www.imanhua.com/recent.html';
			$subcatalogs["排行榜"] = 'http://www.imanhua.com/top.html';

			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$subcatalog = $element->nodeValue;

				//$artist = mb_convert_encoding($artist, "gb2312", "UTF-8");
				//$artist = mb_convert_encoding($artist, "UTF-8", "gb2312");
				//$artist = iconv("GB18030", "UTF-8", $artist);
				if(strlen($href) > 0 && strlen($subcatalog) > 0){
					$subcatalogs[$subcatalog] = 'http://' . $host["host"] . $href;
				}
			}

			$catalog = array();
			$catalog["all"] = $subcatalogs;
			return $catalog;
		}

		function __SearchAuthor($keyword)
		{
			$uri = "http://www.pingshu8.com/bzmtv_inc/SingerSearch.asp?keyword="  . urlencode(iconv("UTF-8", "gb2312", $keyword));
			$response = http_get($uri);
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
			$response = http_get($uri);
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
