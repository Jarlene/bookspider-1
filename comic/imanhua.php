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
		
		function GetChapters($bookid)
		{
			$uri = "http://www.imanhua.com/comic/" . $bookid . "/";
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$xpath = new XPath($response);
			$host = parse_url($uri);

			$iconuri = $xpath->get_attribute("//div[@class='fl bookCover']/img", "src");
			$summary = "";
			$chapters = array();
	
			$i = 0;
			$elements = $xpath->query("//div[@class='intro']/p");
			foreach($elements as $element){
				if($i++ == 0)
					continue; // skip the first catalog

				$p = $element->nodeValue;
				$summary = $summary . $p . "\r\n";
			}

			$elements = $xpath->query("//div[@class='chapterList']/ul/li/a");
			if($elements->length < 1){
				$elements = $xpath->query("//ul[@id='subBookList']/li/a");
			}

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$chapter = $element->getattribute('title');

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

		function GetBooks($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$xpath = new XPath($response);

			$books = array();
			if(0 == strcmp($uri, 'http://www.imanhua.com/recent.html')){
				$elements = $xpath->query("//div[@class='updateList']//li");
				foreach ($elements as $element) {
					$book = $xpath->get_attribute("a", "title", $element, "");
					$href = $xpath->get_attribute("a", "href", $element, "");
					$author = $xpath->get_value("acronym", $element, "");
					$icon = "";
					$time = $xpath->get_value("span", $element, "");
					$status = $xpath->get_value("a[2]", $element, "");

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book, "author" => $author, "icon" => $icon, "time" => $time, "status" => $status);
					}
				}
			} else if(0 == strcmp($uri, 'http://www.imanhua.com/top.html')){
				$elements = $xpath->query("//div[@class='topHits']/ul/li");
				foreach ($elements as $element) {
					$book = $xpath->get_attribute("a", "title", $element, "");
					$href = $xpath->get_attribute("a", "href", $element, "");

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book);
					}
				}
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
					
					$elements = $xpath->query("//ul[@class='bookList']/li");
					foreach ($elements as $element) {
						$book = $xpath->get_attribute("a", "title", $element, "");
						$href = $xpath->get_attribute("a", "href", $element, "");
						$icon = $xpath->get_attribute("a/img", "src", $element, "");
						$time = $xpath->get_attribute("em/a", "title", $element, "");
						$status = $xpath->get_value("em/a", $element, "");

						if(strlen($href) > 0 && strlen($book) > 0){
							$bookid = basename($href);
							$books[$bookid] = array("bookid" => $bookid, "book" => $book, "icon" => $icon, "time" => $time, "status" => $status);
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

			$i = 0;
			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$subcatalog = $element->nodeValue;
				if($i++ == 0)
					continue; // skip the first catalog

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
