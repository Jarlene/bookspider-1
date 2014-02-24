<?php
	class C17TSW
	{
		public $cache = array(
						"catalog" => 2592000, // 30*24*60*60
						"book" => 604800, // 7*24*60*60
						"chapter" => 2592000,
						"audio" => 0,
						"search" => 604800
					);

		public $redirect = 1;

		function GetName()
		{
			return "17tsw";
		}

		function GetAudio($response)
		{
			//file_put_contents ("/app/joke/a.html", $response);
//			$response = http_get($uri);
			//$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=utf-8", $response);
			
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//param[@name='url']");

			foreach ($elements as $element) {
				$href = $element->getattribute('value');
				if(strlen($href) > 0){
					return $href;
//					$uri = iconv("gb2312", "UTF-8", $href);
//					return $uri;
				}
			}

			return "";
		}

		function GetChapters($bookid)
		{
			list($path, $id) = split("-", $bookid);
			$uri = "http://www.17tsw.com/" . $path . "/List_ID_" . $id . ".html";
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$icons = xpath_query($doc, "//div[@class='conlist']/ul/li[1]/img");
			$infos = xpath_query($doc, "//ul[@class='introbox']/p/span");
			$elements = xpath_query($doc, "//ul[@class='compress']/ul/div/li/span/a");

			$host = parse_url($uri);

			$iconuri = "";
			$summary = "";
			foreach($icons as $icon){
				$href = $icon->getattribute('src');
				$iconuri = 'http://' . $host["host"] . $href;
			}
			foreach($infos as $info){
				$summary = $info->nodeValue;
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
					$bookid = basename($href, ".html");
					$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
				}
			}
		}
		
		function GetBooks($uri)
		{
			$books = array();

			if(0 == strcmp($uri, 'http://www.17tsw.com/1')){
				$response = http_get('http://www.17tsw.com/');
				$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				$doc = dom_parse($response);
				$elements = xpath_query($doc, "//div[@id='main']/div[2]/div/ul/li/a");

				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->getattribute('title');

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href, ".html");
						$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
					}
				}
			} else if(0 == strcmp($uri, 'http://www.17tsw.com/2')){
				$response = http_get('http://www.17tsw.com/');
				$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				$doc = dom_parse($response);
				$elements = xpath_query($doc, "//div[@id='main']/div[3]/div/ul/li/a");

				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->getattribute('title');

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href, ".html");
						$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
					}
				}
			} else {
				$response = http_get($uri);
				$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				$doc = dom_parse($response);
				$options = xpath_query($doc, "//div[@class='page']/form/select/option");

				$i = 0;
				foreach ($options as $option) {
					if(0 != $i){
						$u = dirname($uri) . '/' . basename($uri, ".html") . '-' . $i . ".html";
						$response = http_get($u);
						$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
					}
					$this->__ParseBooks($response, $books);
					
					++$i;
					if($i >= 3)
						break;
				}
			}

			$data = array();
			$data["icon"] = "";
			$data["book"] = $books;
			return $data;
		}
		
		function GetCatalog()
		{
			$uri = 'http://www.17tsw.com/';
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@id='nav']/li[@class='menu_test']/a");

			$subcatalogs = array();
			$subcatalogs["最近更新"] = 'http://www.17tsw.com/1';
			$subcatalogs["排行榜"] = 'http://www.17tsw.com/2';

			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$subcatalog = $element->nodeValue;
				if(strlen($href) > 0 && strlen($subcatalog) > 0){
					$subcatalogs[$subcatalog] = 'http://' . $host["host"] . '/' . $href;
				}
			}

			$catalog = array();
			$catalog["小说"] = $subcatalogs;
			return $catalog;
		}
		
		function Search($keyword)
		{
			$uri = 'http://www.17tsw.com/SoClass.aspx';
			$data = 'class=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . '&submit=&ctl00%24Sodaohang=';
			$response = http_post($uri, $data);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$options = xpath_query($doc, "//div[@class='page']/form/select/option");

			$i = 0;
			$books = array();
			foreach ($options as $option) {
				if(0 != $i++){
					$u = 'http://www.17tsw.com/SoClass.aspx?class=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . "&page=" . $i;
					$response = http_get($u);
					$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				}
				$this->__ParseBooks($response, $books);
				break;
			}

			return array("book" => $books);
		}
	}

	// print_r($all);
	//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm'));
	//print_r(ysts8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(_17tsw_chapters('http://www.17tsw.com/LiShiPingShu_cmszygm/List_ID_8473.html'));
	//print_r(_17tsw_audio('http://www.17tsw.com/Play.aspx?id=8473&page=0'));
?>
