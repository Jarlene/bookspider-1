<?php
	require("php/dom.inc");
	require("php/http.inc");
	require("php/util.inc");
	require("http-proxy.php");

	class CVeryIm
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
			return "veryim";
		}

		function GetPictures($uri)
		{
			$html = http_proxy_get($uri, "www.veryim.com", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			$scripts = $xpath->query("/html/head/script");
			if($scripts->length > 2){
				$script = $scripts->item(2)->nodeValue;

				$js = new V8Js();
				$js->executeString("window = this;document={getElementsByTagName:function(){return []}, url:\"\"};");
				$js->executeString($script, "veryim", V8Js::FLAG_FORCE_ARRAY);
				$comic = $js->executeString("comic;", "veryim", V8Js::FLAG_FORCE_ARRAY);
				print_r($comic);
				
				$pictures = array();
				for($i=1; $i <= (int)$comic["totalPage"]; $i++){
					if(1 != (int)$comic["fileNameType"]){
						//var currentImage = comic.imgServer + "/" + comic.letter + "/" + comic.comicDir + "/" + comic.chapterDir + "/" + format(comic.page, comic.fileNameType) + comic.ext, nextImage = "";
						$pictures[] = sprintf('%s/%s/%s/%s/%03d.%s', $comic["imgServer"], $comic["letter"], $comic["comicDir"], $comic["chapterDir"], $i, $comic["ext"]);
					} else {
						$pictures[] = sprintf('%s/%s/%s/%s/%03d%03d.%s', $comic["imgServer"], $comic["letter"], $comic["comicDir"], $comic["chapterDir"], $i, $i-1, $comic["ext"]);
					}
				}
				return $pictures;
			} else {
				return array();
			}
		}

		function GetChapters($bookid)
		{
			$uri = "http://comic.veryim.com/manhua/" . $bookid . "/";
			$html = http_proxy_get($uri, "09019067", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$iconuri = $xpath->get_attribute("//div[@class='cover']/img", "src");

			$summary = "";
			$infos = $xpath->query("//p[@id='summary']");
			foreach($infos as $info){
				foreach($info->childNodes as $node){
					if(XML_TEXT_NODE == $node->nodeType){
						$summary = $summary . $node->nodeValue;
						$summary = $summary . "\r\n";
					}
				}
			}

			$chapters = array();
			$elements = $xpath->query("//div[@id='chapters']/ul/li/a");
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$chapter = $element->nodeValue;

				if(strlen($href) > 0 && strlen($chapter) > 0){
					$chapters[] = array("name" => $chapter, "uri" => $href);
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
			$books = array();
			if(0 == strcmp($uri, '/recent.html')){
				$uri = 'http://www.veryim.com/' . $uri;
				$html = http_proxy_get($uri, "09019067", 10);
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				$xpath = new XPath($html);

				$elements = $xpath->query("//div[@class='newadd']/div[@class='content']/ul/li");
				foreach ($elements as $element) {
					$book = $xpath->get_value("dd/a", $element, "");
					$href = $xpath->get_attribute("dd/a", "href", $element, "");
					$icon = $xpath->get_attribute("dt/a/img", "src", $element, "");;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book, "author" => "", "icon" => $icon, "time" => "", "status" => "");
					}
				}
			} else if(0 == strcmp($uri, '/top.html')){
				$uri = 'http://www.veryim.com/' . $uri;
				$html = http_proxy_get($uri, "09019067", 10);
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				$xpath = new XPath($html);

				$elements = $xpath->query("//div[@class='hot']/ul/li");
				foreach ($elements as $element) {
					$book = $xpath->get_value("a", $element, "");
					$href = $xpath->get_attribute("a", "href", $element, "");

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book);
					}
				}
			} else {
				$uri = 'http://comic.veryim.com' . $uri;
				$html = http_proxy_get($uri, "09019067", 10);
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				$xpath = new XPath($html);

				$page = $xpath->get_value("//div[@class='pages']/span[2]");
				$page = (int)substr($page, 2);
				for($i = 1; $i < $page; $i++) {
					if(1 != $i++){
						$u = "$uri&page=$i";
						$html = http_proxy_get($u, "09019067", 10);
						$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
						$xpath = new XPath($html);
					}

					$elements = $xpath->query("//div[@id='list']/div[@class='list']/ul/li");
					foreach ($elements as $element) {
						$book = $xpath->get_value("div/dl/dt/a/b", $element, "");
						$href = $xpath->get_attribute("div/div/a", "href", $element, "");
						$icon = $xpath->get_attribute("div/div/a/img", "src", $element, "");
						$author = $xpath->get_value("div/dl/dt/a[2]", $element, "");
						$time = $xpath->query("div/dl/dt", $element)->item(0)->nodeValue;

						if(strlen($href) > 0 && strlen($book) > 0){
							$bookid = basename($href);
							$time = substr($time, strpos($time, "20"), 19);
							$books[$bookid] = array("bookid" => $bookid, "book" => $book, "icon" => $icon, "author" => $author, "time" => $time, "status" => "");
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
			$uri = 'http://comic.veryim.com/';
			$html = http_proxy_get($uri, "09019067", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$catalog = array();
			$catalog["最近更新"] = '/recent.html';
			$catalog["排行榜"] = '/top.html';

			$xpath = new XPath($html);
			$elements = $xpath->query("//div[@class='category']/a");
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
			$keyword = "stype=0&keyword=" . urlencode(iconv("UTF-8", "gb2312", $keyword));
			$uri = 'http://www.veryim.com/Search.aspx';
			$html = http_proxy_post($uri, $keyword, "www.veryim.com", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$books = array();
			return $books;
		}
	}
	
// $uri = "http://img1.veryim.com/S/sishen/ch_576/004.png";
// $html = http_get($uri, 20, "", array("Referer: http://comic.veryim.com/manhua/sishen/ch_576.html"));
// file_put_contents ("a.png", $html);

// $veryim = new CVeryIm();
// print_r($veryim->GetCatalog());
// print_r($veryim->GetBooks("/Category.aspx?Id=5"));
// print_r($veryim->GetChapters("haizeiwang"));
// print_r($veryim->GetPictures("http://comic.veryim.com/manhua/haizeiwang/ch_728.html"));
// print_r($veryim->Search("火"));
?>
