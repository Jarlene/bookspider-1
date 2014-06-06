<?php
	class CYSTS8
	{
		public $cache = array(
					"catalog" => 86400,
					"book" => 86400,
					"chapter" => 86400,
					"audio" => 0,
					"search" => 86400
				);

		public $redirect = 0;

		function GetName()
		{
			return "ysts8";
		}

		function GetAudio($bookid, $chapter, $uri)
		{
			$html = http_proxy_get($uri, "Ysjs/bot.js", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			
			$headers = array("Referer: " . $uri);

			$xpath = new XPath($html);
			$uri = $xpath->get_attribute("//iframe[3]", "src");

			$html = http_proxy_get('http://www.ysts8.com/' . $uri, "www.ysts8.com", 20, "proxy.cfg", $headers);

			// http://psf.ysx8.net:8000/ÆäËûÆÀÊé/ÐÂ¶ùÅ®Ó¢ÐÛ´«/001.mp3?
			//file_put_contents ("1.html", $html);
			if(preg_match('/url2.*=.*\'(.*?)\';/', $html, $matches1)){
				if(2 == count($matches1)){
					$uri = $matches1[1];
				}
			}
			
			if(preg_match('/url2\+\'\?(.*?)\'/', $html, $matches)){
			//if(preg_match('/mp3\:\'(.*?)\'/', $html, $matches)){
				if(2 == count($matches)){
					//$arr = explode("$$$", $uri);
					//$pos = strpos($arr[0], "?");
					//$uri = substr($arr[0], $pos+1);
					$uri = $uri . '?' . $matches[1];
					//$uri = $matches[1];
					//return $uri;
					return iconv("gb18030", "UTF-8", $uri);
				}
			}
			return "";
		}

		function GetChapters($bookid)
		{
			$uri = "http://www.ysts8.com/Yshtml/Ys" . $bookid . ".html";
			$response = http_proxy_get($uri, "Ysjs/bot.js", 10);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$infos = xpath_query($doc, "//div[@class='ny_txt']/ul/p");
			$elements = xpath_query($doc, "//div[@class='ny_l']/ul/li/a[1]");

			$summary = "";
			foreach($infos as $info){
				foreach($info->childNodes as $node){
					if(XML_TEXT_NODE == $node->nodeType){
						$summary = $summary . $node->nodeValue;
						$summary = $summary . "\r\n";
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
			$response = http_proxy_get($uri, "Ysjs/bot.js", 10);
			//file_put_contents ("1.html", $html);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			
			$books = array();
			if(0 == strcmp($uri, 'http://www.ysts8.com/index_tim.html')){
				$this->__ParseBooks($u, $response, $books, "//div[@class='pingshu_ysts8_i']/ul/li/a");
			} else if(0 == strcmp($uri, 'http://www.ysts8.com/index_hot.html')){
				$this->__ParseBooks($u, $response, $books, "//div[@class='pingshu_ysts8_i']/ul/li/a");
			} else {
				$options = xpath_query($doc, "//select[@name='select']/option");
				$host = parse_url($uri);
				foreach ($options as $option) {
					$href = $option->getattribute('value');
					if(strlen($href) > 0){
						$u = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $href;
						if(0 != strcmp($u, $uri)){
							$response = http_proxy_get($u, "Ysjs/bot.js", 10);
							$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
						}

						$this->__ParseBooks($u, $response, $books, "//div[@class='pingshu_ysts8']/ul/li/a");
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
			$uri = 'http://www.ysts8.com/index_ys.html';
			$response = http_proxy_get($uri, "Ysjs/bot.js", 10);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			//file_put_contents ("1.html", $response);
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='link']/a");

			$subcatalogs = array();
			$subcatalogs["最近更新"] = 'http://www.ysts8.com/index_tim.html';
			$subcatalogs["排行榜"] = 'http://www.ysts8.com/index_hot.html';

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
			$response = http_proxy_get($uri, "Ysjs/bot.js", 10);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);

			$books = array();
			$n = $this->__SearchPageCount($response);
			for($i=1; $i <= $n; $i++){
				if($i > 1){
					$uri = "http://www.ysts8.com/Ys_so.asp?stype=1&keyword=" . urlencode(iconv("UTF-8", "gb2312", $keyword)) . "page=" . $i;
					$response = http_proxy_get($uri, "Ysjs/bot.js", 10);
					$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				}
				
				$this->__ParseBooks($uri, $response, $books, "//div[@class='pingshu_ysts8']/ul/li/a");
			}
			return array("book" => $books);
		}
	}

	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
	//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
