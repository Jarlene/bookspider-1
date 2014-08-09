<?php
	class CYSTS8
	{
		public $cache = array(
					"catalog" => 864000,    // 10*24*60*60
					"book" => 432000,		// 5*24*60*60
					"chapter" => 259200,	// 3*24*60*60
					"audio" => 0,			 
					"search" => 86400   // 24*60*60
				);

		public $redirect = 0;

		function GetName()
		{
			return "ysts8";
		}

		function GetAudio($bookid, $chapter, $uri)
		{
			global $reply;
			global $headers;
			global $g_redirect;

			$mdb = new Redis();
			$mdb->connect('127.0.0.1', 6379);
			$mdbkey = "ts-server-" . $this->GetName() . "-audio-$bookid-$chapter";
			$mdbvalue = $mdb->get($mdbkey);
			
			if(0 == $g_redirect){
				// first request
				$reply["code"] = 300;
				$reply["bookid"] = $bookid;
				$reply["chapterid"] = $chapter;

				if(!$mdbvalue){
					$audio["uri"] = $uri;
					$audio["flv"] = "";
					$mdb->set($mdbkey, json_encode($audio));

					$headers["Referer"] = "http://www.ysts8.com/Yshtml/Ys$bookid.html";
					return $uri;
				} else {
					$audio = json_decode($mdbvalue, True);
					if(strlen($audio["flv"]) < 1){
						$headers["Referer"] = "http://www.ysts8.com/Yshtml/Ys$bookid.html";
						return $uri;
					} else {
						$headers["Referer"] = $audio["uri"];
						return $audio["flv"];
					}
				}
			} else {
				// client submit data
				$html = $uri;
				$audio = json_decode($mdbvalue, True);
				//file_put_contents("a.html", $html);
				if(strlen($audio["flv"]) < 1){
					$xpath = new XPath($html);
					for($i = 1; $i < 5; $i++){
						$flv = $xpath->get_attribute("//iframe[$i]", "src");
						if(stripos($flv, 'flv.asp?url=http'))
							break;
					}
					//$flv = $xpath->get_attribute("//iframe[1]", "src");
					if(strlen($flv) > 0){
						$flv = "http://www.ysts8.com" . $flv;
						//file_put_contents ("b.html", $flv);
						$audio["flv"] = $flv;
						$mdb->set($mdbkey, json_encode($audio));
					} else {
						$flv = $audio["uri"];
					}

					$reply["code"] = 300;
					$reply["bookid"] = $bookid;
					$reply["chapterid"] = $chapter;
					$headers["Referer"] = $audio["uri"];
					return $flv;
				} else {
					//file_put_contents ("a.html", $html);
					// 6102986163870x1406030845x6103448776624-5be9cd2016294cd6a07a0a063876fdbc
					if(!preg_match('/([0-9]+x140[0-9]{7}x[0-9]+)/', $html, $matches1)){
						print_r("don't match time");
						return "";
					}

					if(!preg_match('/([0-9a-fA-F]{16,})/', $html, $matches2)){
						print_r("don't match hash");
						return "";
					}

					if(2 == count($matches1)){
						$postfix = $matches1[1];
					}

					$uri = $audio["flv"];
					$arr = explode("&", $uri);
					$arr = explode("?", $arr[0]);
					$arr = explode("=", $arr[1]);
					$uri = urldecode($arr[1]);
					$uri = iconv("gb18030", "UTF-8", $uri);

					//$t = time();
					//$postfix = sprintf("?%ux%ux%u-f6441157ba03c991857d77880d9f9f9e", $t+218070220011, $t, $t+462612754+218070220011);
					$uri = $uri . '?' . $postfix . '-' . $matches2[1];
					return $uri;

					//if(preg_match_all('/url2\+\'\?(.*?)\'/', $html, $matches)){
					if(preg_match('/mp3\:\'\'\+(.*?)\+\'(.*?)\'/', $html, $matches)){
					//if(preg_match('/\+\'\?(.*?)\'/', $html, $matches)){
						if(3 == count($matches)){
							print_r($matches[1]);
							print_r($matches[2]);
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
			}
			print_r("what's wrong?\n");
			return "";
		}
		
		function GetAudio2($bookid, $chapter, $uri)
		{
			$html = http_proxy_get($uri, "Ysjs/bot.js", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$headers = array("Referer: " . $uri);

			$xpath = new XPath($html);
			$uri = $xpath->get_attribute("//iframe[1]", "src");
			$html = http_proxy_get('http://www.ysts8.com/' . $uri, "www.ysts8.com", 20, "proxy.cfg", $headers);
			
			//file_put_contents ("a.html", $html);
			// 6102986163870x1406030845x6103448776624-5be9cd2016294cd6a07a0a063876fdbc
			if(!preg_match('/([0-9]+x140[0-9]{7}x[0-9]+)/', $html, $matches1)){
                print_r("don't match time");
				return "";
			}
			
			if(!preg_match('/([0-9a-fA-F]{16,})/', $html, $matches2)){
                print_r("don't match hash");
                return "";
			}

			if(2 == count($matches1)){
				$postfix = $matches1[1];
			}
			// if(strpos($html, "jp-jplayer") == false){
				// $uri = $xpath->get_attribute("//iframe[2]", "src");
				// $html = http_proxy_get('http://www.ysts8.com/' . $uri, "www.ysts8.com", 20, "proxy.cfg", $headers);
			// }

			//file_put_contents ("a.html", $html);
			// if(preg_match('/url2.*=.*\'(.*?)\';/', $html, $matches1)){
				// if(2 == count($matches1)){
					// $uri = $matches1[1];
				// }
			// }
			$arr = explode("&", $uri);
			$arr = explode("?", $arr[0]);
			$arr = explode("=", $arr[1]);
			$uri = urldecode($arr[1]);
			$uri = iconv("gb18030", "UTF-8", $uri);

			//$t = time();
			//$postfix = sprintf("?%ux%ux%u-f6441157ba03c991857d77880d9f9f9e", $t+218070220011, $t, $t+462612754+218070220011);
			return $uri . '?' . $postfix . '-' . $matches2[1];

			//if(preg_match_all('/url2\+\'\?(.*?)\'/', $html, $matches)){
			if(preg_match('/mp3\:\'\'\+(.*?)\+\'(.*?)\'/', $html, $matches)){
			//if(preg_match('/\+\'\?(.*?)\'/', $html, $matches)){
				if(3 == count($matches)){
					print_r($matches[1]);
					print_r($matches[2]);
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
