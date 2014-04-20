<?php
	require("php/dom.inc");
	require("php/http.inc");
	require("php/util.inc");
	require("http-proxy.php");

	class CXXBH
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
			return "xxbh";
		}

		function GetPictures($uri)
		{
			$html = http_proxy_get($uri, "template/xxbh", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			$scripts = $xpath->query("/html/head/script", null);
			if($scripts->length > 1){
				$script = $scripts->item(1)->nodeValue;

				$js = new V8Js();
				$js->executeString($script, "xxbh", V8Js::FLAG_FORCE_ARRAY);
				$base64 = $js->executeString("qTcms_S_m_murl_e;", "xxbh", V8Js::FLAG_FORCE_ARRAY);
				$url = base64_decode($base64);
				$urls = explode('$qingtiandy$', $url);
				return $urls;
			} else {
				return array();
			}
		}

		function GetChapters($bookid)
		{
			$uri = "http://www.xxbh.net/comic/" . $bookid;
			$html = http_proxy_get($uri, "template/xxbh", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$iconuri = $xpath->get_attribute("//div[@class='l21']/img", "src");

			$i = 0;
			$infos = $xpath->query("//div[@class='l21']/p");

			$summary = "";
			foreach($infos as $info){
				foreach($info->childNodes as $node){
					if(XML_TEXT_NODE == $node->nodeType){
						$summary = $summary . $node->nodeValue;
						$summary = $summary . "\r\n";
					}
				}
			}

			$host = parse_url($uri);
			$chapters = array();

			$elements = $xpath->query("//ul[@id='ul-b-d-div']/li/a");
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
			$uri = 'http://www.xxbh.net' . $uri;
			$html = http_proxy_get($uri, "template/xxbh", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);

			$books = array();
			if(0 == strcmp($uri, 'http://www.xxbh.net/comicone/page_a.html')){
				$elements = $xpath->query("//ul[@class='ul222']/li");
				foreach ($elements as $element) {
					$href = $xpath->get_attribute("a[2]", "href", $element, "");
					$book = $xpath->get_attribute("a/img", "alt", $element, "");
					$icon = $xpath->get_attribute("a/img", "src", $element, "");
					$time = $xpath->get_value("em", $element, "");
					$status = $xpath->get_value("b", $element, "");

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book, "icon" => $icon, "time" => $time, "status" => $status);
					}
				}
			} else if(0 == strcmp($uri, 'http://www.xxbh.net/comicone/page_b.html')){
				$elements = $xpath->query("//ul[@class='ul_list']/li");
				foreach ($elements as $element) {
					$book = $xpath->get_attribute("a", "title", $element, "");
					$href = $xpath->get_attribute("a", "href", $element, "");

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href);
						$books[$bookid] = array("bookid" => $bookid, "book" => $book);
					}
				}
			} else {
				$page = $xpath->get_attribute("//a[@id='k_2']", "href");
				$page = (int)$page;

				for($i = 1; $i <= $page; $i++) {
					if(1 != $i){
						$u = $uri . "$i.html";
						$html = http_proxy_get($u, "template/xxbh", 10);
						$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
						$xpath = new XPath($html);
					}
					
					$elements = $xpath->query("//ul[@class='ul222']/li");
					foreach ($elements as $element) {
						$href = $xpath->get_attribute("a[2]", "href", $element, "");
						$book = $xpath->get_attribute("a/img", "alt", $element, "");
						$icon = $xpath->get_attribute("a/img", "src", $element, "");
						$time = $xpath->get_value("em", $element, "");
						$status = $xpath->get_value("b", $element, "");

						if(strlen($href) > 0 && strlen($book) > 0){
							$bookid = basename($href);
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
			$uri = 'http://www.xxbh.net/';
			$html = http_proxy_get($uri, "template/xxbh", 10);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$catalog = array();
			$catalog["最近更新"] = '/comicone/page_a.html';
			$catalog["排行榜"] = 'comicone/page_b.html';

			$xpath = new XPath($html);
			$elements = $xpath->query("//ul[@class='ul4']/li/a");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$text = $element->nodeValue;

				if(strlen($href) > 1 && strlen($text) > 0){
					$catalog[$text] = $href;
				}
			}

			return $catalog;
		}
	}

// $xxbh = new CXXBH();
// print_r($xxbh->GetCatalog());
// print_r($xxbh->GetBooks("/xuanyi/"));
// print_r($xxbh->GetChapters("12841.html"));
// print_r($xxbh->GetPictures("http://www.xxbh.net/comic/12841/126530.html"));
// print_r($xxbh->Search("火"));
?>
