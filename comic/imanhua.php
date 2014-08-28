<?php
require_once("php/dom.inc");
require_once("phttp.php");

class CIManHua 
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
		$html = $this->http->get($uri, "/template/default/foot.js");
		$html = str_replace("charset=gb2312", "charset=gb18030", $html);
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
	
	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function ListBook()
	{
		$comics = array();
		$uri = "http://www.imanhua.com/all.html";
		$html = $this->http->get($uri, 'foot.js');
		$html = str_replace("charset=gb2312", "charset=gb18030", $html);
		if(strlen($html) < 1) return $comics;

		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='allComicList']/ul/li");
		foreach ($elements as $element) {
			$comic = array();
			$comic["name"] = $xpath->get_attribute("a[1]", "title");
			$comic["href"] = $xpath->get_attribute("a[1]", "href");
			$comic["icon"] = $xpath->get_attribute("a[1]", "rel");
			$comic["chapter"] = $xpath->get_attribute("a[2]", "title");

			$id = basename($comic["href"]);
			$comics[$id] = $comic;
		}

		return $comics;
	}
	
	function GetBook($bookid)
	{
		$uri = "http://www.imanhua.com/comic/$bookid/";
		$html = $this->http->get($uri, 'foot.js');
		$html = str_replace("charset=gb2312", "charset=gb18030", $html);
		if(strlen($html) < 1) return False;
		// file_put_contents("imanhua-$bookid.html", $html);

		$xpath = new XPath($html);
		$icon = $xpath->get_attribute("//div[@class='bookCover']/img | //div[@class='fl bookCover']/img", "src");

		$summary = "";
		$elements = $xpath->query("//div[@class='intro']/p");
		foreach($elements as $element){
			$text = $element->nodeValue;
			$summary = $summary . $text . "\r\n";
		}

		$elements = $xpath->query("//div[@class='chapterList']/ul/li/a");
		if($elements->length < 1){
			$elements = $xpath->query("//ul[@id='subBookList']/li/a");
		}

		$chapters = array();
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$chapter = $element->getattribute('title');

			if(strlen($href) > 0 && strlen($chapter) > 0){
				$chapters[] = array("name" => $chapter, "uri" => 'http://www.imanhua.com' . $href);
			}
		}

		$header = $xpath->get_value("//p[@class='cf bookAttr'] | //p[@class='bookAttr']");
		$headers = explode("|", $header);
		if(4 != count($headers)){
			print_r("GetBook($bookid): get bookAttr failed.\n");
			die();
		}

		//  完结状态：[ 连载中 ] 原作者：尾田荣一郎 | 字母索引：A | 加入时间：2007-04-30 | 更新时间：2014-08-20 
		$author = substr($headers[0], strpos($headers[0], "原作者：")+strlen("原作者："));
		$intime = substr($headers[2], strpos($headers[2], "加入时间：")+strlen("加入时间："));
		$uptime = substr($headers[3], strpos($headers[3], "更新时间：")+strlen("更新时间："));
		$status = strcmp("连载中" , substr($headers[0], strpos($headers[0], "完结状态：[ ")+strlen("完结状态：[ ")));

		$book = array();
		$book["icon"] = "http://www.imanhua.com" . $icon;
		$book["author"] = trim($author);
		$book["status"] = $status;
		$book["catalog"] = "";
		$book["tags"] = "";
		$book["region"] = "";
		$book["datetime"] = $uptime . " 00:00:00"; // "Y-m-d H:i:s";
		$book["summary"] = $summary;
		$book["section"] = array("name" => "", "chapters" => $chapters);
		return $book;
	}

	function GetChapter($bookid, $chapterid)
	{
		list($bname, $bid) = explode("_", $bookid);
		if(strlen($bname) > 0){
			$uri = "http://www.imanhua.com/comic/$bid/$bname$chapterid.shtml";
		} else {
			$uri = "http://www.imanhua.com/comic/$bid/list_$chapterid.html";
		}

		$html = $this->http->get($uri, "foot_chapter.js");
		$html = str_replace("charset=gb2312", "charset=gb18030", $html);
		if(strlen($html) < 1) return False;
		//file_put_contents("imanhua-$bookid-$chapterid.html", $html);
		//$html = file_get_contents("imanhua-$bookid-$chapterid.html");

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
	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function __construct($proxy="proxy.cfg1")
	{
		$this->http = new PHttp($proxy);
		$this->http->get_http()->settimeout(120);
	}

	private $http;
}

// $uri = "http://c4.mangafiles.com/Files/Images/76/61224/imanhua_001.png";
// $html = http_get($uri, 20, "", array("Referer: http://www.imanhua.com/comic/76/list_61224.html"));
// file_put_contents ("a.png", $html);

// $site = new CIManHua();
// print_r($site->GetCatalog());
// print_r($site->ListBook());
// print_r($site->GetBook("1083"));
// print_r($site->GetChapter("_1083", "29914")); // http://www.imanhua.com/comic/1083/list_29914.html
// print_r($site->GetPictures("http://www.imanhua.com/comic/5018/list_90310.html"));
// print_r($site->search("火"));
?>
