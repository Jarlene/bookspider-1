<?php
require_once("php/dom.inc");
require_once("php/http.inc");

class CBenGou
{
	function ListBook()
	{
		$comics = array();

		$uri = "http://www.bengou.cm/all/-1----scorecount-16-2/index.html";
		$html = $this->__http_get($uri, "email.gif");
		$html = str_replace("<head>", '<head><meta http-equiv="content-type" content="text/html; charset=utf-8" />', $html);

		if(strlen($html) < 1) return $comics;

		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='sa-comic_introlist']/ul/li");
		foreach ($elements as $element) {
			$comic = array();
			$comic["icon"] = $xpath->get_attribute("div[@class='pic']/a/img", "src", $element);
			$comic["uri"] = $xpath->get_attribute("//h2[@class='title']/a", "href", $element);
			$comic["name"] = $xpath->get_attribute("//h2[@class='title']/a", "title", $element);
			$comic["author"] = $xpath->get_value("//div[@class='comic_tag_container']/a[1]", $element);
			$comic["status"] = $xpath->get_value("//div[@class='comic_tag_container']/a[2]", $element);
			$comic["date"] = $xpath->get_value("//p[@class='date_status']/span[2]", $element);

			$comic["id"] = basename($comic["icon"], ".jpg");
			$comic["status"] = 0==strcmp("连载", $comic["status"]);
			$comics[] = $comic;
		}

		return $comics;
	}

	function GetBook($bookid)
	{
		$uri = "http://www.bengou.cm/cartoon/douluodalu/";
		$html = $this->__http_get($uri, "email.gif");
		$html = str_replace("<head>", '<head><meta http-equiv="content-type" content="text/html; charset=utf-8" />', $html);
		if(strlen($html) < 1) return False;

		$sections = array();
		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='section-list mark']");
		foreach ($elements as $element) {
			$chapters = array();
			$nodes = $xpath->query("span/a", $element);
			foreach ($nodes as $node) {
				$href = $node->getattribute("href");
				$name = $node->nodeValue;
				$chapters[] = array("name" => $name, "href" => $href);
			}

			$section = array();
			$section["name"] = $xpath->get_value("h6", $element);
			$section["chapters"] = $chapters;
			$sections[] = $section;
		}

		$book = array();
		$book["icon"] = $xpath->get_attribute("//div[@class='cartoon-intro']/a/img", "src");
		$book["author"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[1]/a");
		$book["status"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[2]");
		$book["catalog"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[3]/a");
		$book["region"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[5]/a");
		$book["datetime"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[6]");
		$book["summary"] = $xpath->get_value("//p[@id='cartoon_digest2']");
		$book["section"] = $sections;
		return $book;
	}

	function GetChapter($bookid, $chapterid)
	{
		$uri = "http://www.bengou.cm/cartoon/douluodalu/1_101957.html";
		$html = $this->__http_get($uri, "email.gif");
		$html = str_replace("<head>", '<head><meta http-equiv="content-type" content="text/html; charset=utf-8" />', $html);
		if(strlen($html) < 1) return False;

		$baseuri = "";
		//var pic_base = 'http://img.bengou.cm:82';
		if(preg_match('/var pic_base = \'(.+?)\';/', $html, $matches)){
			if(2 == count($matches)){
				$baseuri = trim($matches[1], "\'");
			}
		}
		if(strlen($baseuri) < 1) return False;

		$chapters = array();
		//'\/file\/diyimg\/2013\/8\/29\/lf2qf4kxcc1374344865.jpg'
		if(preg_match('/var picTree =\[(.+?)\];/', $html, $matches)){
			if(2 == count($matches)){
				$v = $matches[1];
				$v = str_replace("\\", "", $v);
				$i = 1;
				$pictures = explode(",", $v);
				foreach ($pictures as $picture) {
					$picture = trim($picture, "\'");
					$chapters[] = array("uri" => $baseuri . $picture, "referer" => $uri . "_$i");
					++$i;
				}
			}
		}

		return $chapters;
	}

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function __construct($proxy="proxy.cfg")
	{
		$this->http = new Http();
		$this->http->setcookie("/var/cookie.bengou");
		$this->http->settimeout(120);

		$this->proxies = split(",", file_get_contents($proxy));
	}

	private function __http_get($uri, $pattern, $headers=array())
	{
		static $idx = -1;

		if(count($this->proxies) > 0){
			return $this->http->get($uri, $headers);
		} else {
			if(-1 == $idx)
			{
				$idx = rand() % count($this->proxies);
				$this->http->setproxy($this->proxies[$idx]);
			}

			for($i = 0; $i < 5 && $i < count($this->proxies); $i++){
				$html = $this->http->get($uri, $headers);
				if(stripos($html, $pattern)){
					return $html;
				} else {
					unset($this->proxies[$idx]);
				}

				if(count($this->proxies) > 0){
					$idx = ($idx + 1) % count($this->proxies);
					print_r("[$idx] " . $this->proxies[$idx] . "\n");
					$this->http->setproxy($this->proxies[$idx]);
				}
			}
		}

		return "";
	}

	private $http;
	private $proxies;
}

//$site = new CBenGou();
//print_r($site->GetBook(""));
//print_r($site->GetChapter("", ""));
?>
