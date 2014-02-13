<?php
	class CYSTS8
	{
		function GetName()
		{
			return "ysts8";
		}

		function GetAudio($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			
			if(!preg_match('/\"\/play\/flv\.html\?(.+?)\"/', $response, $matches)){
				return "";
			}

			return 2 == count($matches) ? iconv("gb2312", "UTF-8", $matches[1]) : "";
		}

		function GetChapters($bookid)
		{
			$uri = "http://www.ysts8.com/Yshtml/Ys" . $bookid . ".html";
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$infos = xpath_query($doc, "//div[@class='ny_txt']/ul/p");
			$elements = xpath_query($doc, "//div[@class='ny_l']/ul/li/a[1]");

			$summary = "";
			if(is_null($infos)){
				print_r("parse book icon/information error.");
			} else {
				foreach($infos as $info){
					foreach($info->childNodes as $node){
						if(XML_TEXT_NODE == $node->nodeType){
							$summary = $summary . $node->nodeValue;
							$summary = $summary . "\r\n";
						}
					}
				}
			}

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

			$data = array();
			$data["icon"] = "";
			$data["info"] = $summary;
			$data["chapter"] = $chapters;
			return $data;
		}

		function __ParseBooks($uri, $response, &$books)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='pingshu_ysts8']/ul/li/a");

			if (!is_null($elements)) {
				$host = parse_url($uri);
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->firstChild->wholeText;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href, ".html");
						$books[substr($bookid, 2)] = $book;
					}
				}
			}
		}

		function GetBooks($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$options = xpath_query($doc, "//select[@name='select']/option");

			$books = array();

			if (!is_null($options)) {
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
			} else {
				$this->__ParseBooks($uri, $response, $books);
			}
			
			$data = array();
			$data["icon"] = "";
			$data["book"] = $books;
			return $data;
		}
		
		function GetCatalog()
		{
			$uri = 'http://www.ysts8.com/index_ys.html';
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='link']/a");

			$subcatalogs = array();

			if (!is_null($elements)) {
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
			}

			$catalog = array();
			$catalog["小说"] = $subcatalogs;
			return $catalog;
		}

		function __SearchPageCount($response)
		{
			$doc = dom_parse($response);
			$pages = xpath_query($doc, "//ul[@class='pagelist']/table/tr/td/center/font/b/a[4]");
			
			if (!is_null($pages)) {
				foreach ($pages as $page) {
					$href = $page->getattribute('href');
					if(strlen($href) > 0){
						if(1 == sscanf($href, "page=%d", $n))
							return $n;
					}
				}
			}
			return 1;
		}
		
		function Search($keyword)
		{
			$uri = "http://www.ysts8.com/Ys_so.asp?stype=1&keyword=". urlencode(iconv("UTF-8", "gb2312", $keyword));
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);

			$books = array();
			$n = $this->__SearchPageCount($response);
			for($i=1; $i <= $n; $i++){
				if($i > 1){
					$uri = "http://www.ysts8.com/Ys_so.asp?stype=1&keyword=" . urlencode(iconv("UTF-8", "gb2312", $keyword)) . "page=" . $i;
					$response = http_get($uri);
					$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				}
				
				$this->__ParseBooks($uri, $response, $books);
			}
			return array("book" => $books);
		}
	}

	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
	//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
