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

		function GetChapters($uri)
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

		function __ParseBooks($uri, $response)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='pingshu_ysts8']/ul/li/a");

			$books = array();

			if (!is_null($elements)) {
				$host = parse_url($uri);
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->firstChild->wholeText;

					if(strlen($href) > 0 && strlen($book) > 0){
						$books[$book] = 'http://' . $host["host"] . $href;
					}
				}
			}

			return $books;
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

						$nbooks = $this->__ParseBooks($u, $response);
						$books = array_merge($books, $nbooks);
					}
				}
			} else {
				$nbooks = $this->__ParseBooks($uri, $response);
				$books = array_merge($books, $nbooks);
			}
			
			return $books;
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
	}

	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
	//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
