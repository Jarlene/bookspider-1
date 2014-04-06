<?php	
class CPingShu8
{
	public $cache = array(
					"catalog" => 604800, // 7*24*60*60
					"book" => 604800,
					"chapter" => 604800,
					"audio" => 0,
					"search" => 86400 // 24*60*60
				);

	public $redirect = 0;

	function GetName()
	{
		return "pingshu8";
	}

	function GetAudio($bookid, $chapter, $uri)
	{
		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);
		$mdbkey = "ts-server-" . $this->GetName() . "-audio-$bookid-$chapter";
		$rawuri = $mdb->get($mdbkey);
		if(!$rawuri){
			$uri = $this->__GetAudio($uri);
			if($uri){
				$rawuri = substr($uri, 0, strpos($uri, '?'));
				$mdb->set($mdbkey, $rawuri);
			}
		} else {
			// $mp3 = basename($rawuri);
			// $ps = strrpos($mp3, '_');
			// $pe = strrpos($mp3, '.');
			// $cid = substr($mp3, $ps+1, $pe-$ps-1);
			// $n = strlen($cid);
			 // if(2 == $n) {
				// $cidNew = sprintf("%02d", $chapter);
			// } else if(3 == $n) {
				// $cidNew = sprintf("%03d", $chapter);
			// } else if(4 == $n) {
				// $cidNew = sprintf("%04d", $chapter);
			// } else {
				// $cidNew = sprintf("%d", $chapter);
			// }
			
			$ip = $this->__ip();
			$ip = str_replace(".", "0", $ip);
			
			$t = time();
			$postfix = sprintf("?%ux%ux%u-6618f00ff155173c7dddb190142ace21", $t+$ip, $t, $t+5778742+$ip);

			$uri = $rawuri . $postfix;
			$uri = str_replace("pl0.", "p0a1.", $uri);
			$uri = str_replace("pl1.", "p1a1.", $uri);
		}
		
		return $uri;
	}
	
	function __GetAudio($uri)
	{
		$bookid = basename($uri);
		$n = strpos($bookid, '_');
		$id = substr($bookid, $n+1);
		$uri = "http://www.pingshu8.com/path_" . $id;
		//$html = http_proxy_post($uri, "", "luckyzz@163.com", 10, "proxy.cfg", array("Referer: http://www.pingshu8.com/play_161404.html"));
		$html = http_proxy_post($uri, "", ":8000", 10, "proxy.cfg", array("Referer: http://www.pingshu8.com/play_161404.html"));
		//$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		$obj = json_decode($html);
		//var_dump($obj);
		//file_put_contents ("a.html", $obj->{"urlpath"});

		// if(!preg_match('/encodeURI\(\"(.+)\"\)/', $html, $matches)){
			// return "";
		// }

		// if(2 != count($matches)){
			// return "";
		// }

		//$uri = $matches[1];
		$uri = $obj->{"urlpath"};
		$n = strrpos($uri, '?');
		$uri = substr($uri, 0, $n);

		//$uri = str_replace("@123abcd", "9", $uri);
		$uri = str_replace(".flv", ".mp3", $uri);
		$uri = str_replace("play0.", "p0a1.", $uri);
		$uri = str_replace("play1.", "p1a1.", $uri);

		$ip = $this->__ip();
		$ip = str_replace(".", "0", $ip);

		$t = time();
		$postfix = sprintf("?%ux%ux%u-6618f00ff155173c7dddb190142ace21", $t+$ip, $t, $t+5778742+$ip);
		
		$uri = $uri . $postfix;
		//return iconv("gb18030", "UTF-8", $uri);
		return $uri;
	}

	function GetChapters($bookid)
	{
		$uri = "http://www.pingshu8.com/MusicList/mmc_" . $bookid . ".htm";
		$html = http_proxy_get($uri, "luckyzz@163.com", 10);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1){
			$data = array();
			$data["icon"] = "";
			$data["info"] = "";
			$data["chapter"] = array();
			return $data;
		}

		$host = parse_url($uri);
		$xpath = new XPath($html);

		$iconuri = $xpath->get_attribute("//div[@class='a']/img", "src");
		if(0==strncmp("../", $iconuri, 3)){
			$iconuri = 'http://' . $host["host"] . dirname(dirname($host["path"])) . '/' . substr($iconuri, 3);
		} else {
			$iconuri = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $iconuri;
		}

		$summary = $xpath->get_value("//div[@class='c']/div");

		$chapters = array();

		$pages = array();
		$selects = $xpath->query("//select[@name='turnPage']/option");
		foreach ($selects as $select) {
			$href = $select->getattribute('value');
			if(strlen($href) > 0){
				$u = 'http://' . $host["host"] . $href;
				if(0 != strcasecmp($u, $uri)){
					$pages[] = $u;
				} else {
					$result = $this->__ParseChapters($html);
					foreach($result as $chapter){
						$uri = 'http://' . $host["host"] . $chapter["uri"];
						$chapters[] = array("name" => $chapter["name"], "uri" => $uri);
					}
				}
			}
		}

		if(count($pages) > 0){
			$result = array();
			$http = new HttpMultipleProxy("proxy.cfg");
			$r = $http->get($pages, array($this, '_OnReadChapter'), &$result, 20);

			if(count($result) != count($pages)){
				assert(0 != $r);
				$chapters = array(); // empty data(some uri request failed)
			} else {
				for($i = 0; $i < count($result); $i++){
					foreach($result[$i] as $chapter){
						$uri = 'http://' . $host["host"] . $chapter["uri"];
						$chapters[] = array("name" => $chapter["name"], "uri" => $uri);
					}
				}
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
		$iconuri = "";

		$html = http_proxy_get($uri, "luckyzz@163.com", 10);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1){
			$data = array();
			$data["icon"] = $iconuri;
			$data["book"] = $books;
			return $data;
		}

		$xpath = new XPath($html);

		if(0 == strcmp($uri, 'http://www.pingshu8.com/music/newzj.htm')){
			$elements = $xpath->query("//div[@class='tab3']/ul/li[2]/a[2]");
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->nodeValue;

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href);
					$n = strpos($bookid, '.');
					$books[substr($bookid, 4, $n-4)] = $book;
				}
			}
		} else if(0 == strcmp($uri, 'http://www.pingshu8.com/top/pingshu.htm')){
			$elements = $xpath->query("//div[@class='tab3']/a");
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->nodeValue;

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href);
					$n = strpos($bookid, '.');
					$books[substr($bookid, 4, $n-4)] = $book;
				}
			}
		} else {
			$iconuri = $xpath->get_attribute("//div[@class='z4']/img", "src");
			$elements = $xpath->query("//div[@class='jj2']/div/div/a");

			if(0==strncmp("../", $iconuri, 3)){
				$iconuri = 'http://' . $host["host"] . dirname(dirname($host["path"])) . '/' . substr($iconuri, 3);
			} else {
				$iconuri = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $iconuri;
			}

			$host = parse_url($uri);
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->nodeValue;

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href);
					$n = strpos($bookid, '.');
					$books[substr($bookid, 4, $n-4)] = $book;
				}
			}
		}

		$data = array();
		$data["icon"] = $iconuri;
		$data["book"] = $books;
		return $data;
	}
	
	function GetCatalog()
	{
		$catalog = array();
		$catalog["评书"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_1.Htm');
		$catalog["相声小品"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_2.Htm');
		$catalog["小说"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_3.Htm');
		$catalog["金庸全集"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_4.Htm');
		$catalog["综艺娱乐"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_5.Htm');
		return $catalog;
	}

	function Search($keyword)
	{
		$authors = $this->__SearchAuthor($keyword);
		$books = $this->__SearchBook($keyword);
		return array("catalog" => $authors, "book" => $books);
	}

	//---------------------------------------------------------------------------
	// private function
	//---------------------------------------------------------------------------
	function _OnReadChapter($param, $i, $r, $header, $body)
	{
		if(0 != $r){
			//error_log("_OnReadChapter $i: error: $r\n", 3, "pingshu.log");
			return -1;
		} else if(!stripos($body, "luckyzz@163.com")){
			// check html content integrity
			//error_log("Integrity check error $i\n", 3, "pingshu.log");
			return -1;
		}

		$param[$i] = $this->__ParseChapters($body);
		return 0;
	}

	private function __ParseChapters($html)
	{
		$chapters = array();

		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $chapters;
		
		$xpath = new XPath($html);
		$elements = $xpath->query("//li[@class='a1']/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$chapter = $element->nodeValue;

			if(strlen($href) > 0 && strlen($chapter) > 0){
				$chapters[] = array("name" => $chapter, "uri" => $href);
			}
		}

		return $chapters;
	}

	private function __GetSubcatalog($uri)
	{
		$artists = array();
		$artists["最近更新"] = 'http://www.pingshu8.com/music/newzj.htm';
		$artists["排行榜"] = 'http://www.pingshu8.com/top/pingshu.htm';

		$html = http_proxy_get($uri, "luckyzz@163.com", 10);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $artists;

		$host = parse_url($uri);
		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='t2']/ul/li/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$artist = $element->nodeValue;

			//$artist = mb_convert_encoding($artist, "gb2312", "UTF-8");
			//$artist = mb_convert_encoding($artist, "UTF-8", "gb2312");
			//$artist = iconv("GB18030", "UTF-8", $artist);
			if(strlen($href) > 0 && strlen($artist) > 0){
				$artists[$artist] = 'http://' . $host["host"] . $href;
			}
		}

		return $artists;
	}
	
	private function __SearchAuthor($keyword)
	{
		$artists = array();

		$uri = "http://www.pingshu8.com/bzmtv_inc/SingerSearch.asp?keyword="  . urlencode(iconv("UTF-8", "gb2312", $keyword));
		$html = http_proxy_get($uri, "luckyzz@163.com", 5);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $artists;

		$xpath = new XPath($html);
		$elements = $xpath->query("//table[@class='TableLine']/form/tr/td[1]/div/a");
		foreach ($elements as $element) {
			$artist = $element->nodeValue;
			if(strlen($artist) > 0){
				$artists[] = $artist;
			}
		}

		return $artists;
	}

	private function __SearchBook($keyword)
	{
		$books = array();

		$uri = "http://www.pingshu8.com/bzmtv_inc/SpecialSearch.asp?keyword=" . urlencode(iconv("UTF-8", "gb2312", $keyword));
		$html = http_proxy_get($uri, "luckyzz@163.com", 5);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $books;

		$host = parse_url($uri);
		$xpath = new XPath($html);
		$elements = $xpath->query("//table[@class='TableLine']/tr/td[1]/div/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$book = $element->nodeValue;

			$bookid = basename($href);
			$n = strpos($bookid, '.');
			$books[substr($bookid, 4, $n-4)] = $book;
		}
		return $books;
	}
	
	private function __ip()
	{
		if (getenv("HTTP_CLIENT_IP"))
			$ip = getenv("HTTP_CLIENT_IP");
		else if(getenv("HTTP_X_FORWARDED_FOR"))
			$ip = getenv("HTTP_X_FORWARDED_FOR");
		else if(getenv("REMOTE_ADDR"))
			$ip = getenv("REMOTE_ADDR");
		else 
			$ip = "0.0.0.0";
		return $ip;
	}
}

// require("php/dom.inc");
// require("php/util.inc");
// require("php/http.inc");
require("php/http-multiple.inc");
require("http-proxy.php");
require("http-multiple-proxy.php");	
// $obj = new CPingShu8();
// print_r($obj->GetCatalog()); sleep(2);
// print_r($obj->GetBooks("http://www.pingshu8.com/Special/Msp_7.Htm")); sleep(2);
// print_r($obj->GetChapters('7_208_1')); sleep(2);
// print_r($obj->GetAudio('7_208_1', '1', "http://www.pingshu8.com/play_19632.html")); sleep(2);
// print_r($obj->Search("单田芳")); sleep(2);
// print_r($obj->__GetAudio("http://www.pingshu8.com/play_27123.html"));
// print_r($obj->GetAudio("138_252_1", "8", "http://www.pingshu8.com/play_23860.html"));
?>
