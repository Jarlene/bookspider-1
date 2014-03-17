<?php
	class C77NT
	{
		public $cache = array(
							"catalog" => 2592000,
							"book" => 604800,
							"chapter" => 2592000,
							"audio" => 2592000,
							"search" => 604800
						);

		public $redirect = 0;

		function GetName()
		{
			return "77nt";
		}

		function GetAudio($bookid, $chapter, $uri)
		{
			$uri = str_replace("Play", "zyurl", $uri);
			return $uri;
			$headers = http_get_headers($uri, "Location");

			if(!preg_match("/Location:([^\r\n]*)/i", $headers, $matches)){
				return "";
			}

			return 2 == count($matches) ? iconv("gb2312", "UTF-8", $matches[1]) : "";
		}

		function GetChapters($bookid)
		{
			list($path, $id) = split("-", $bookid);
			$uri = "http://www.77nt.com/" . $path . "/List_ID_" . $id . ".html";
			$html = http_get($uri);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			
			$xpath = new XPath($html);
			$icons = $xpath->query("//div[@class='conlist']/ul/li[1]/img");
			$infos = $xpath->query("//ul[@class='introbox']/p/span");
			$elements = $xpath->query("//ul[@class='compress']/ul/div/li/span/a");

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

		function GetBooks($uri)
		{
			$books = array();

			if(0 == strcmp($uri, 'http://www.77nt.com/1')){ // update
				$html = http_get('http://www.77nt.com/');
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

				$xpath = new XPath($html);
				$elements = $xpath->query("//div[@id='main']/div[2]/div/ul/li/a");
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->getattribute('title');

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href, ".html");
						$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
					}
				}
			} else if(0 == strcmp($uri, 'http://www.77nt.com/2')){ // top
				$html = http_get('http://www.77nt.com/');
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

				$xpath = new XPath($html);
				$elements = $xpath->query("//div[@id='main']/div[3]/div/ul/li/a");
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->getattribute('title');

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = basename($href, ".html");
						$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
					}
				}
			} else { // books
				$html = http_get($uri);
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

				$xpath = new XPath($html);
				$options = $xpath->query("//div[@class='page']/form/select/option");

				$pages = array();
				$items = array();
				foreach ($options as $option) {
					$value = $option->getattribute('value');
					if($value != '0')
						$pages[] = dirname($uri) . '/' . basename($uri, ".html") . '-' . $value . ".html";
				}
				
				print_r($pages);
				
				// page 1
				$result = array();
				$this->__ParseBooks($html, &$result);

				// other pages
				if(count($pages) > 0){
					$http = new HttpMultipleProxy("proxy.cfg");
					$r = $http->get($pages, array($this, '_OnReadBook'), &$result, 60);
					if(0 != $r){
						// log error
						//$items = array(); // empty data(some uri request failed)
					} else {
						$books = $result;
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
			$uri = 'http://www.77nt.com/';
			$html = http_proxy_get($uri);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			$xpath = new XPath($html);
			$elements = $xpath->query("//div[@id='nav']/li[@class='menu_test']/a");

			$subcatalogs = array();
			$subcatalogs["最近更新"] = 'http://www.77nt.com/1';
			$subcatalogs["排行榜"] = 'http://www.77nt.com/2';

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
			$uri = 'http://www.77nt.com/SoClass.aspx';
			$data = 'class=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . '&submit=&ctl00%24Sodaohang=';
			$html = http_post($uri, $data);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			$options = $xpath->query("//div[@class='page']/form/select/option");

			$i = 0;
			$books = array();
			foreach ($options as $option) {
				if(0 != $i++){
					$u = 'http://www.77nt.com/Soclass.aspx?class=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . "&page=" . $i;
					$html = http_get($u);
					$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				}
				$this->__ParseSearch($html, $books);
			}

			return array("book" => $books);
		}
		
		//---------------------------------------------------------------------------
		// private function
		//---------------------------------------------------------------------------
		function _OnReadBook($param, $i, $r, $header, $body)
		{
			if(0 != $r){
				//error_log("_OnReadBook $i: error: $r\n", 3, "77nt.log");
				//print_r("_OnReadBook error: $i: $r\n");
				return -1;
			} else if(!stripos($body, "copy.js")){
				// check html content integrity
				//error_log("Integrity check error $i\n", 3, "77nt.log");
				//print_r("Integrity check error $i\n");
				return -1;
			}

			$this->__ParseBooks($body, &$param);
			return 0;
		}

		private function __ParseBooks($html, &$books)
		{
			$xpath = new XPath($html);
			$elements = $xpath->query("//div[@class='clist']/ul/li/a");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->getattribute('title');

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".html");
					$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
				}
			}
		}
		
		private function __ParseSearch($response, &$books)
		{
			$doc = dom_parse($response);
			$elements = xpath_query($doc, "//div[@class='clist3']/ul/li/a");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->getattribute('title');

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".html");
					$books[basename(dirname($href)) . '-' . substr($bookid, 8)] = $book;
				}
			}
		}
	}

require("php/dom.inc");
require("php/util.inc");
require("php/http.inc");
require("php/http-multiple.inc");
require("http-proxy.php");
require("http-multiple-proxy.php");

$obj = new C77NT();
// print_r($obj->GetCatalog()); sleep(2);
print_r($obj->GetBooks("http://www.77nt.com/DouFuXiaoShui/DouFuXiaoShui.html")); sleep(2);
// print_r($obj->GetChapters('DouFuXiaoShui-8436')); sleep(2); // http://www.77nt.com/DouFuXiaoShui/List_ID_8436.html
// print_r($obj->GetAudio('DouFuXiaoShui-8436', '1', "http://www.77nt.com/Play.aspx?id=8436&page=0")); sleep(2);
// print_r($obj->Search("单田芳")); sleep(2);
?>
