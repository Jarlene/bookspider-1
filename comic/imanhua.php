<?php
	require("php/dom.inc");
	require("php/http.inc");
	require("php/util.inc");
	require("http-proxy.php");

	class CIManHua 
	{
		function GetName()
		{
			return "imanhua";
		}

		function GetPictures($uri)
		{
			$html = http_proxy_get($uri, "footer-main", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			$scripts = $xpath->query("/html/head/script");
			if($scripts->length > 0){
				$script = $scripts->item(0)->nodeValue;

				$js = new V8Js();
				$js->executeString($script, "imanhua", V8Js::FLAG_FORCE_ARRAY);
				$cInfo = $js->executeString("cInfo;", "imanhua", V8Js::FLAG_FORCE_ARRAY);

				//$servers = array('c5.mangafiles.com', 'c4.mangafiles.com', 't5.mangafiles.com', 't4.mangafiles.com');

				if($cInfo["cid"] > 7910){
					// http://www.imanhua.com/comic/76/list_61224.html
					// http://c4.mangafiles.com/Files/Images/76/61224/imanhua_001.png
					// "/Files/Images/"+cInfo.bid+"/"+cInfo.cid+"/"+$cInfo["files"][$i]
					$pictures = array();
					foreach($cInfo["files"] as $file){
						$pictures[] = "http://c4.mangafiles.com" . "/Files/Images/" . $cInfo["bid"] . "/" . $cInfo["cid"] . "/" . $file;
					}
					return $pictures;
				} else {
					// http://www.imanhua.com/comic/135/list_7198.html
					// "/pictures/135/7198/trdh01.jpg"
					foreach($cInfo["files"] as $file){
						$pictures[] = "http://t4.mangafiles.com" . $file;
					}
					return $cInfo["files"];
				}
			} else {
				return array();
			}
		}

		function GetChapters($bookid)
		{
			$uri = "http://www.imanhua.com/comic/" . $bookid . "/";
			$html = http_proxy_get($uri, "footer", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$iconuri = $xpath->get_attribute("//div[@class='fl bookCover']/img", "src");

			$i = 0;
			$elements = $xpath->query("//div[@class='intro']/p");

			$summary = "";
			foreach($elements as $element){
				if($i++ == 0) continue; // skip the first catalog

				$text = $element->nodeValue;
				$summary = $summary . $text . "\r\n";
			}

			$elements = $xpath->query("//div[@class='chapterList']/ul/li/a");
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
			$data["summary"] = $summary;
			$data["chapter"] = $chapters;
			return $data;
		}

		function GetBooks($uri)
		{
			$uri = 'http://www.imanhua.com' . $uri;
			$html = http_proxy_get($uri, "/template/default/foot.js", 10);
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
				$page = $xpath->get_value("//div[@class='pagerHead']/strong[3]", null, 1);

				$host = parse_url($uri);
				for($i = 1; $i <= $page; $i++) {
					if(1 != $i){
						$u = $uri . 'index_p' . $i . '.html';
						$html = http_proxy_get($u, "/template/default/foot.js", 10);
						$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
						$xpath = new XPath($html);
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
							$time = substr($time, strpos($time, "20")); // year 2014/2013
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
			$html = http_proxy_get($uri, "footer", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$catalog = array();
			$catalog["最近更新"] = '/recent.html';
			$catalog["排行榜"] = '/top.html';

			$xpath = new XPath($html);
			$elements = $xpath->query("//ul[@class='navList']/li/a");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$text = $element->nodeValue;

				if(strlen($href) > 1 && strlen($text) > 0){
					$catalog[$text] = $href;
				}
			}

			return $catalog;
		}

		function Search($keyword)
		{
			$keyword = urlencode(iconv("UTF-8", "gb2312", $keyword));
			$uri = 'http://www.imanhua.com/v2/user/search.aspx?key=' . $keyword;
			$html = http_proxy_get($uri, "footer", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$page = $xpath->get_value("//div[@class='pagerHead']/strong[3]", null, 1);

			$books = array();
			$host = parse_url($uri);
			for($i = 1; $i <= $page; $i++) {
				if(1 != $i){
					$u = "http://www.imanhua.com/v2/user/search.aspx?key=$keyword&p=$i";
					$html = http_proxy_get($u, "footer", 10);
					$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
					$xpath = new XPath($html);
				}

				$elements = $xpath->query("//div[@class='bookChrList']/ul/ul/li");
				foreach ($elements as $element) {
					$book = $xpath->get_attribute("div[@class='intro']/h2/a", "title", $element, "");
					$href = $xpath->get_attribute("div[@class='intro']/h2/a", "href", $element, "");
					$icon = $xpath->get_attribute("div[@class='cover']/a/img", "src", $element, "");
					$time = $xpath->get_attribute("div[@class='intro']/em/a", "title", $element, "");
					$status = $xpath->get_value("div[@class='intro']/em/a", $element, "");

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$time = substr($time, strpos($time, ":")+1);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book, "icon" => $icon, "time" => $time, "status" => $status);
					}
				}
			}

			return $books;
		}
	}

// $uri = "http://c4.mangafiles.com/Files/Images/76/61224/imanhua_001.png";
// $html = http_get($uri, 20, "", array("Referer: http://www.imanhua.com/comic/76/list_61224.html"));
// file_put_contents ("a.png", $html);

// $imanhua = new cimanhua();
// print_r($imanhua->GetCatalog());
// print_r($imanhua->GetBooks("/comic/tuili/"));
// print_r($imanhua->GetChapters("5018")); // http://www.imanhua.com/comic/5018/
// print_r($imanhua->GetPictures("http://www.imanhua.com/comic/5018/list_90310.html"));
// print_r($imanhua->search("火"));
?>
