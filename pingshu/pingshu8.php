<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");

	class CPingShu8 
	{
		function GetName()
		{
			return "pingshu8";
		}

		function GetAudio($uri)
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

			return iconv("gb2312", "UTF-8", $uri);
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
						$chapters[$chapter] = 'http://' . $host["host"] . $href;
					}
				}
			}

			return $chapters;
		}
		
		function GetChapters($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$icons = xpath_query($doc, "//div[@class='a']/img");
			$infos = xpath_query($doc, "//div[@class='c']/div");
			$options = xpath_query($doc, "//select[@name='turnPage']/option");

			$host = parse_url($uri);
			$iconuri = "";
			$summary = "";
			if(is_null($icons) || is_null($infos)){
				print_r("parse book icon/information error.");
			} else {
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

		function GetBooks($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$icons = xpath_query($doc, "//div[@class='z4']/img");
			$elements = xpath_query($doc, "//div[@class='jj2']/div/div/a");

			$host = parse_url($uri);
			$iconuri = "";

			if(is_null($icons)){
				print_r("parse books icon error.");
			} else {
				foreach($icons as $icon){
					$href = $icon->getattribute('src');
					if(0==strncmp("../", $href, 3)){
						$iconuri = 'http://' . $host["host"] . dirname(dirname($host["path"])) . '/' . substr($href, 3);
					} else {
						$iconuri = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $href;
					}
				}
			}

			$books = array();

			if (!is_null($elements)) {
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					if(strlen($href) > 0 && strlen($book) > 0){
						$books[$book] = 'http://' . $host["host"] . $href;
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
			$elements = $xpath->query("//table[@class='TableLine']/tr");

			$books = array();
			if (!is_null($elements)) {
				$host = parse_url($uri);
				foreach ($elements as $element) {
					$author = $xpath->query("td[2]/a", $element);
					$book = $xpath->query("td[1]/div/a", $element);
					if(0==$author->length || 0==$book->length)
						continue;

					$books[] = array("catalog" => $author->item(0)->nodeValue, "book" => $book->item(0)->nodeValue);
				}
			}
			return $books;
		}

		function Search($keyword)
		{
			$authors = __SearchAuthor($keyword);
			$books = __SearchBook($keyword);
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
