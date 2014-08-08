<?php
	class CBenGou
	{
		function GetChapters($bookid)
		{
			$uri = "http://www.imanhua.com/comic/" . $bookid . "/";
			$html = http_proxy_get($uri, "footer", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$iconuri = $xpath->get_attribute("//div[@class='bookInfo']/div[@class='img']/img", "src");

			$summary = "";
			$infos = $xpath->query("//div[@class='bookInfo']/dl/dd/p[2]");
			foreach($infos as $info){
				foreach($info->childNodes as $node){
					if(XML_TEXT_NODE == $node->nodeType){
						$summary = $summary . $node->nodeValue;
						$summary = $summary . "\r\n";
					}
				}
			}

			$elements = $xpath->query("//div[@class='bookList']/ul/li/a");
			if($elements->length < 1){
				$elements = $xpath->query("//ul[@id='subBookList']/li/a");
			}

			$host = parse_url($uri);
			$chapters = array();
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
			$uri = 'http://bengou.co' . $uri;
			$html = http_proxy_get($uri, "footer", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$books = array();
			if(0 == strcmp($uri, 'http://www.imanhua.com/recent.html')){
				$elements = $xpath->query("//div[@class='updateList']//li");
				foreach ($elements as $element) {
					$book = $xpath->get_attribute("a", "title", $element, "");
					$href = $xpath->get_attribute("a", "href", $element, "");
					$author = $xpath->get_value("acronym", $element, "");
					$icon = "";
					$time = $xpath->get_value("span", $element, "");
					$status = $xpath->get_attribute("a[2]", "title", $element, "");

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
				$i = 1;
				$host = parse_url($uri);

				$pages = $xpath->query("//div[@class='page']/select/option");				
				foreach ($pages as $page) {
					if(1 != $i++){
						$u = 'http://bengou.co' . $xpath->get_attribute(".", "value", $page, "");
						$html = http_proxy_get($u, "BenGou", 10);
						$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
						$xpath = new XPath($html);
					}

					$elements = $xpath->query("//div[@class='scmtList']/ul/li");
					foreach ($elements as $element) {
						$book = $xpath->get_attribute("a", "title", $element, "");
						$href = $xpath->get_attribute("a", "href", $element, "");
						$icon = $xpath->get_attribute("a/img", "src", $element, "");

						if(strlen($href) > 0 && strlen($book) > 0){
							$bookid = basename($href);
							$books[$bookid] = array("bookid" => $bookid, "book" => $book, "icon" => $icon, "time" => "", "status" => "");
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
			$uri = 'http://bengou.co/';
			$html = http_proxy_get($uri, "BenGou", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$subcatalogs = array();
			$subcatalogs["最近更新"] = '/recent.html';
			$subcatalogs["排行榜"] = '/top.html';

			$xpath = new XPath($html);
			$elements = $xpath->query("//div[@class='main_nav']/ul/li[@class='mini']/a");
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$text = $element->nodeValue;

				if(strlen($href) > 1 && strlen($text) > 0){
					$subcatalogs[$text] = $href;
				}
			}

			$catalog = array();
			$catalog["all"] = $subcatalogs;
			return $catalog;
		}
	}
?>
