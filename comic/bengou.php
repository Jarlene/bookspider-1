<?php
require_once("php/dom.inc");
require_once("phttp.php");

class CBenGou
{
	public static $timeout = 0;
	public static $siteid = 1;

	function ListBook()
	{
		$count = 1;
		$comics = array();
		for($i = 0; $i < (int)$count; $i++){
			$uri = sprintf('http://www.bengou.cm/all/-%d----scorecount-16-2/index.html', $i+1);
			$html = $this->http->get($uri, "email.gif");
			$html = str_replace("<head>", '<head><meta http-equiv="content-type" content="text/html; charset=utf-8" />', $html);
			if(strlen($html) < 1) continue;
	
			$xpath = new XPath($html);
			if(0 == $i){
				$v = $xpath->get_attribute("//div[@class='mod-page']/a[last()]", "href");
				list($count) = sscanf($v, "/all/-%d----"); // update page count
			}

			$elements = $xpath->query("//div[@class='sa-comic_introlist']/ul/li");
			foreach ($elements as $element) {
				$uri = $xpath->get_attribute(".//h2[@class='title']/a", "href", $element);
				$status = $xpath->get_value(".//div[@class='comic_tag_container']/a[2]", $element);
				$date = $xpath->get_value(".//p[@class='date_status']/span[2]", $element);
				list($year, $mon, $day) = explode(".", $date);

				$comic = array();
				$comic["name"] = $xpath->get_attribute(".//h2[@class='title']/a", "title", $element);
				$comic["icon"] = $xpath->get_attribute("div[@class='pic']/a/img", "src", $element);
				$comic["author"] = $xpath->get_value(".//div[@class='comic_tag_container']/a[1]", $element);
				$comic["status"] = 0==strcmp("连载", $status);
				$comic["date"] = date("Y-m-d H:i:s", mktime(0, 0, 0, $mon, $day, $year));
				$comic["summary"] = "";

				$id = basename($uri) . "_" . basename($comic["icon"], ".jpg");
				$comics[$id] = $comic;
			}

			sleep(CBenGou::$timeout);
		}
		return $comics;
	}

	function GetBook($bookid)
	{
		//$uri = "http://www.bengou.cm/cartoon/douluodalu/";
		list($bname, $bid) = explode("_", $bookid);
		$uri = sprintf('http://bengou.cm/cartoon/%s/', $bname);
		$html = $this->http->get($uri, "email.gif");
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
				//http://bengou.cm/cartoon/xiudougaoxiao/7278_134945.html
				list($bid, $cid) = explode("_", basename($href, ".html"));
				$chapters[] = array("name" => $name, "id" => $cid);
			}

			$section = array();
			$section["name"] = $xpath->get_value("h6", $element);
			$section["chapters"] = $chapters;
			$sections[] = $section;
		}

		$datetime = $xpath->get_value("//div[@class='cartoon-intro']/div/p[6]");
		list($year, $mon, $day, $h, $m, $s) = sscanf($datetime, "更新时间：%d/%d/%d %d:%d:%d");

		$book = array();
		$book["icon"] = $xpath->get_attribute("//div[@class='cartoon-intro']/a/img", "src");
		$book["author"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[1]/a");
		$book["status"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[2]");
		$book["catalog"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[3]");
		$book["tags"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[4]");
		$book["region"] = $xpath->get_value("//div[@class='cartoon-intro']/div/p[5]/a");
		$book["datetime"] = date("Y-m-d H:i:s", mktime($h, $m, $s, $mon, $day, $year));;
		$book["summary"] = $xpath->get_value("//p[@id='cartoon_digest2']");
		$book["section"] = $sections;
		return $book;
	}

	function GetChapter($bookid, $chapterid)
	{
		//$uri = "http://www.bengou.cm/cartoon/douluodalu/1_101957.html";
		list($bname, $bid) = explode("_", $bookid);
		$uri = sprintf('http://bengou.cm/cartoon/%s/%d_%d.html', $bname, $bid, $chapterid);
		$html = $this->http->get($uri, "html");
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
					$chapters[] = array("uri" => $baseuri . $picture, "referer" => dirname($uri) . "/" . basename($uri, ".html") . "_$i.html");
					++$i;
				}
			}
		}

		return $chapters;
	}

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function __construct($proxy="proxy.cfg1")
	{
		$this->http = new PHttp($proxy);
		//$this->http->get_http()->setcookie("/var/cookie.bengou");
		$this->http->get_http()->settimeout(120);
	}

	private $http;
}

//$site = new CBenGou();
//print_r($site->ListBook());
//print_r($site->GetBook("douluodalu_1"));
//print_r($site->GetChapter("douluodalu_1", "101957"));
?>
