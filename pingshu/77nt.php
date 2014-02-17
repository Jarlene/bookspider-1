<?php
	class C77NT
	{
		function GetName()
		{
			return "77nt";
		}

		function GetAudio($uri)
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

				print_r($book);
				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".html");
					$books[dirname($href) . '-' . substr($bookid, 8)] = $book;
				}
			}
		}
		
		function GetBooks($uri)
		{
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$options = xpath_query($doc, "//div[@class='page']/form/select/option");

			$i = 1;
			$books = array();
			foreach ($options as $option) {
				if(1 != $i++){
					$u = dirname($uri) . '/' . basename($href, ".html") . $i . ".html";
					$response = http_get($u);
					$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				}
				$this->__ParseBooks($u, $response, $books);
				
				break;
			}

			$data = array();
			$data["icon"] = "";
			$data["book"] = $books;
			return $data;
		}

		function GetCatalog()
		{
			$uri = 'http://www.77nt.com/';
			$response = http_get($uri);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@id='nav']/li[@class='menu_test']/a");

			$subcatalogs = array();

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

		function __ParseSearch($response, &$books)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='clist3']/ul/li/a");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->getattribute('title');

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".html");
					$books[dirname($href) . '-' . substr($bookid, 8)] = $book;
				}
			}
		}
		
		function Search($keyword)
		{
			$uri = 'http://www.77nt.com/SoClass.aspx';
			$data = 'class=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . '&submit=&ctl00%24Sodaohang=';
			$response = http_post($uri, $data);
			$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
			$doc = dom_parse($response);
			$options = xpath_query($doc, "//div[@class='page']/form/select/option");

			$i = 0;
			$books = array();
			foreach ($options as $option) {
				if(0 != $i++){
					$u = 'http://www.77nt.com/Soclass.aspx?class=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . "&page=" . $i;
					$response = http_get($u);
					$response = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $response);
				}
				$this->__ParseSearch($u, $response, $books);
			}

			return array("book" => $books);
		}
	}

	// print_r($all);
	//print_r(_77nt_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(_77nt_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(_77nt_chapters('http://www.77nt.com/LiShiPingShu/List_ID_8333.html'));
	//print_r(_77nt_audio('http://www.77nt.com/Play.aspx?id=9184&page=0'));
?>
