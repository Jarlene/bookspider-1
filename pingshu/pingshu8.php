<?php	

require_once("db-pingshu.inc");

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
	public $useDelegate = 1;
	public $dbhost = "127.0.0.1";

	function GetName()
	{
		return "pingshu8";
	}

	
	function GetAudio($bookid, $chapter, $uri)
	{
		global $headers;
		
		//list($play, $chapterid) = explode("_", basename($uri, ".html"));
		$chapterid = $uri;

		$rawuri = $this->DBGetAudio($bookid, $chapterid);
//		if(strlen($rawuri) < 1)
//			$rawuri = $this->WebGetAudio($bookid, $chapterid, $uri);
		return $rawuri;
	}

	function __EncodeAudioURI($uri)
	{
		$n = strrpos($uri, '?');
		$suffix = substr($uri, $n+1);
		$uri = substr($uri, 0, $n);

		list($v1, $v0, $v2) = sscanf($suffix, "%ux%ux%u-");

		$t = time();
		$ip = $this->__ip();
		$ip = str_replace(".", "0", $ip);
		$postfix = sprintf("?%ux%ux%u-6618f00ff155173c7dddb190142ace21", $t+$ip, $t, $t+$v2-$v1+$ip);

		$uri = $uri . $postfix;
		// $uri = iconv("gb18030", "UTF-8", $uri);
		return $uri;
	}
	
	function WebGetAudio($bookid, $chapter, $uri)
	{
		global $reply;
		global $headers;
		global $g_redirect;

		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);
		$mdbpath = "ts-server-" . $this->GetName() . "-audio-post-path";

		if(0 == $g_redirect){
			$chapter = $uri; // get chapter from uri
			$mdbkey = "ts-server-" . $this->GetName() . "-audio-$bookid-$chapter";

			// first request
			$mdbvalue = $mdb->get($mdbkey);
			if(!$mdbvalue){
				$reply["code"] = 300;
				$reply["bookid"] = $bookid;
				$reply["chapterid"] = $chapter;

				$seed = mt_rand();
				//if(0 == seed % 9){
				if(True){
					// some user to help us update post path value
					$headers["Referer"] = "http://www.pingshu8.com/MusicList/mmc_$bookid.html";
					$uri = "http://www.pingshu8.com/play_$chapter.html";		
					return $uri;
				} else {
					$path = "path_";
					$mdbvalue = $mdb->get($mdbpath);
					if($mdbvalue)
						$path = $mdbvalue;

					$headers["Referer"] = "http://www.pingshu8.com/play_$chapter.html";
					$uri = "http://www.pingshu8.com/$path$chapter.html";		
					return $uri;
				}
			} else {
				$headers["Referer"] = "http://www.pingshu8.com/play_$chapter.html";
				$headers["Accept"] = "audio/webm,audio/ogg,audio/wav,audio/*;q=0.9,application/ogg;q=0.7,video/*;q=0.6,*/*;q=0.5";
				$uri = $this->__EncodeAudioURI($mdbvalue);
			}
		} else {
			// client submit data
			$mdbkey = "ts-server-" . $this->GetName() . "-audio-$bookid-$chapter";

			$html = $uri;
			if(stripos($html, 'luckyzz@163.com')) {
				// play_xxx.html
				if(preg_match_all('/src=\"\/Play_Flash\/js\/(.+)\"/', $html, $matches)){
					if(2 == count($matches)){
						$uri = $matches[1][2];

						$reply["code"] = 300;
						$reply["bookid"] = $bookid;
						$reply["chapterid"] = $chapter;
						//$headers["Referer"] = "http://www.pingshu8.com/play_$chapter.html";
						//$headers["Referer"] = sprintf("http://www.pingshu8.com/play_%s.html", $chapter);
						return "http://www.pingshu8.com/Play_Flash/js/$uri";
					}
				}
				print_r("don't find xxx.js\n");
			} else if(False !== stripos($html, 'document.domain = "pingshu8.com"')){
				// xxx.js
				// url: "SYKTIO7K" + urlpath + ".html",
				if(preg_match('/url: \"(.+?)\"/', $html, $matches)){
					if(2 == count($matches)){
						$path = $matches[1];
						if(strlen($path) > 0)
							$mdb->set($mdbpath, $path);

						$reply["code"] = 300;
						$reply["bookid"] = $bookid;
						$reply["chapterid"] = $chapter;
						//$headers["Referer"] = "http://www.pingshu8.com/play_$chapter.html";
						$uri = "http://www.pingshu8.com/$path$chapter.html";
						return $uri;
					}
				}
				print_r("don't find path pattern\n");
			} else {
				//file_put_contents("a.html", $html);
				$obj = json_decode($html);
				$uri = $obj->{"urlpath"};

				if(strlen($uri) > 0)
				{
					$uri = str_replace("@123abcd", "9", $uri);
					$uri = str_replace(".flv", ".mp3", $uri);
					$mdb->set($mdbkey, $uri, 24*60*60);
				}

				$headers["Referer"] = "http://www.pingshu8.com/play_$chapter.html";
				$headers["Accept"] = "audio/webm,audio/ogg,audio/wav,audio/*;q=0.9,application/ogg;q=0.7,video/*;q=0.6,*/*;q=0.5";
				$uri = $this->__EncodeAudioURI($uri);
			}
		}

		return $uri;
	}
	
	function WebGetAudio2($bookid, $chapterid)
	{
		$uri = "http://www.pingshu8.com/play_$chapterid.html";
		$referer = "Referer: http://www.pingshu8.com/MusicList/mmc_$bookid.htm";
		$html = http_proxy_get($uri, "luckyzz@163.com", 10, "proxy.cfg", array($referer));
		//$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

		if(preg_match('/encodeURI\(\"(.+)\"\)/', $html, $matches)){
			$uri = $matches[1];
		} else {
			$encrystr = "";
			if(preg_match('/var encrystr =\"(.+)\"/', $html, $matches)){
				$encrystr = $matches[1];
			}

			$posturi = "http://www.pingshu8.com/path_$chapterid.html";
			$referer = "Referer: http://www.pingshu8.com/play_$chapterid.html";
			$postdata = "encrystr=$encrystr&urlpath=$chapterid";
			$html = http_proxy_post($posturi, $postdata, ":8000", 10, "proxy.cfg", array($referer));
			$obj = json_decode($html);
			//file_put_contents ("a.html", $obj->{"urlpath"});
			$uri = $obj->{"urlpath"};
		}

		// $n = strrpos($uri, '?');
		// $uri = substr($uri, 0, $n);
		// $suffix = substr($uri, $n+1);
		$uri = str_replace("@123abcd", "9", $uri);
		$uri = str_replace(".flv", ".mp3", $uri);
		// $uri = str_replace("play0.", "p0a1.", $uri);
		// $uri = str_replace("play1.", "p1a1.", $uri);
		// return iconv("gb18030", "UTF-8", $uri);
		return $uri;
	}

	function DBGetAudio($bookid, $chapterid)
	{
		$db = new DBPingShu($this->dbhost);
		$chapter = $db->get_chapter($bookid, $chapterid);
		if(False === $chapter)
			return "";
		$uri = $chapter["uri"];

		$server = "";
		$path = "/";
		$urls = explode(":", $uri);
		if(count($urls) == 1){
			$server = "115.28.51.131";
			$path = $uri;
		} else {
			$server = $urls[0];
			$path = $urls[1];
		}

		if(strlen($path) < 1)
			return "";

		$uri = $path;
		$uri = str_replace("//home/pingshu8", "/1", $uri);
		$uri = str_replace("/home/pingshu8", "/1", $uri);
		$uri = str_replace("//ts/pingshu8", "/2", $uri);
		$uri = str_replace("/ts/pingshu8", "/2", $uri);
		$uri = str_replace("//ts2/pingshu8", "/3", $uri);
		$uri = str_replace("/ts2/pingshu8", "/3", $uri);
		return "http://$server$uri";
	}

	//----------------------------------------------------------------------------
	// GetChapters
	//----------------------------------------------------------------------------
	function GetChapters($bookid)
	{
		$data = $this->DBGetChapters($bookid);
		if(False === $data)
			return $this->WebGetChapters($bookid);
		return $data;
	}

	function DBGetChapters($bookid)
	{
		$db = new DBPingShu($this->dbhost);
		$book = $db->get_book($bookid);
		$chapters = $db->get_chapters($bookid);
		if(False == $book || 0 == count($chapters))
			return False;
		
		$data = array();
		$data["icon"] = $book["icon"];
		$data["info"] = $book["summary"];
		$data["count"] = count($chapters);
		$data["chapter"] = array();
		$data["catalog"] = $book["catalog"];
		$data["subcatalog"] = $book["subcatalog"];
		foreach($chapters as $id => $value){
			$data["chapter"][] = array("name" => $value["name"], "uri" => $id);
		}
	
		return $data;
	}

	function WebGetChapters($bookid)
	{
		$book = $this->GetBookInfo($bookid);

		list($v1, $v2, $v3) = explode("_", $bookid);

		$urls = array();
		$page = $book["page"];
		for($i = 1; $i < $page; $i++){
			$urls[] = sprintf("http://www.pingshu8.com/MusicList/mmc_%d_%d_%d.htm", $v1, $v2, $i+1);
		}

		if(count($urls) > 0){
			$result = array();
			if($this->useDelegate)
				$http = new HttpMultipleProxy("proxy.cfg");
			else
				$http = new HttpMultiple();
			$r = $http->get($urls, array($this, '_OnReadChapter'), &$result, 20);

			if(count($result) != count($urls)){
				assert(0 != $r);
				$book["chapter"] = array(); // empty data(some uri request failed)
			} else {
				for($i = 0; $i < count($result); $i++){
					foreach($result[$i] as $chapter){
						$book["chapter"][] = $chapter;
					}
				}
			}
		}

		return $book;
	}

	function GetBookInfo($bookid)
	{
		$uri = "http://www.pingshu8.com/MusicList/mmc_$bookid.htm";
		$referer = "Referer: http://www.pingshu8.com/top/xiangsheng.htm";
		if($this->useDelegate)
			$html = http_proxy_get($uri, "luckyzz@163.com", 20, "proxy.cfg", array($referer));
		else
			$html = http_get($uri, 20, "", array($referer));
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1){
			$data = array();
			$data["icon"] = "";
			$data["info"] = "";
			$data["page"] = 0;
			$data["count"] = 0;
			$data["chapter"] = array();
			$data["catalog"] = "";
			$data["subcatalog"] = "";
			return $data;
		}

		$host = parse_url($uri);
		$xpath = new XPath($html);
		$value = $xpath->get_value("//div[@class='list5']/div");
		$selects = $xpath->query("//select[@name='turnPage']/option");
		$iconuri = $xpath->get_attribute("//div[@class='a']/img", "src");

		if(0==strncmp("../", $iconuri, 3)){
			$data["icon"] = 'http://' . $host["host"] . dirname(dirname($host["path"])) . '/' . substr($iconuri, 3);
		} else {
			$data["icon"] = 'http://' . $host["host"] . dirname($host["path"]) . '/' . $iconuri;
		}

		list($count) = sscanf($value, " 共有%d集");
		$data["info"] = $xpath->get_value("//div[@class='c']/div");
		$data["page"] = $selects->length;
		$data["count"] = $count;
		$data["chapter"] = $this->ParseChapter($html);
		$data["catalog"] = $xpath->get_value("//div[@class='t1']/div/a[2]");
		$data["subcatalog"] = $xpath->get_value("//div[@class='t1']/div/a[3]");
		return $data;
	}
	
	//----------------------------------------------------------------------------
	// GetBooks
	//----------------------------------------------------------------------------
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

		$host = parse_url($uri);
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
		} else if(0 == strcmp($uri, 'http://www.pingshu8.com/top/pingshu.htm')
			|| 0 == strcmp($uri, 'http://www.pingshu8.com/top/yousheng.htm')
			|| 0 == strcmp($uri, 'http://www.pingshu8.com/top/xiangsheng.htm')
			|| 0 == strcmp($uri, 'http://www.pingshu8.com/top/zongyi.htm')){
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
		$catalog["评书"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_1.Htm', 'http://www.pingshu8.com/top/pingshu.htm', '评书排行榜');
		$catalog["相声小品"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_2.Htm', 'http://www.pingshu8.com/top/xiangsheng.htm', '相声小品排行榜');
		$catalog["小说"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_3.Htm', 'http://www.pingshu8.com/top/yousheng.htm', '小说排行榜');
		$catalog["金庸全集"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_4.Htm', 'http://www.pingshu8.com/Special/Msp_218.Htm', '金庸作品排行榜');
		$catalog["综艺娱乐"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_5.Htm', 'http://www.pingshu8.com/top/zongyi.htm', '综艺娱乐排行榜');
		return $catalog;
	}

	function Search($keyword)
	{
		$db = new DBPingShu($this->dbhost);
		$dbbooks = $db->search($keyword);
		if(False === $dbbooks)
			return array("catalog" => array(), "book" => array());

		$books = array();
		foreach ($dbbooks as $bookid => $value) {
			$books[$bookid] = $value["name"];
		}

		//$authors = $this->__SearchAuthor($keyword);
		//$books = $this->__SearchBook($keyword);
		return array("catalog" => array(), "book" => $books);
	}

	function ParseChapter($html)
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
				list($play, $chapterid) = explode("_", basename($href, ".html"));
				$chapters[] = array("name" => $chapter, "uri" => $chapterid);
			}
		}

		return $chapters;
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

		$param[$i] = $this->ParseChapter($body);
		return 0;
	}

	function __GetSubcatalog($uri, $uritop, $topname)
	{
		$artists = array();
		$artists["最近更新"] = 'http://www.pingshu8.com/music/newzj.htm';
		$artists[$topname] = $uritop;

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
// require("php/http-multiple.inc");
// require("http-proxy.php");
// require("http-multiple-proxy.php");
// $obj = new CPingShu8();
// print_r($obj->GetCatalog()); sleep(2);
// print_r($obj->GetBooks("http://www.pingshu8.com/Special/Msp_7.Htm")); sleep(2);
// print_r($obj->GetChapters('33_239_1')); sleep(2);
// print_r($obj->GetAudio('7_208_1', '1', "http://www.pingshu8.com/play_19632.html")); sleep(2);
// print_r($obj->Search("单田芳")); sleep(2);
// print_r($obj->__GetAudio("http://www.pingshu8.com/play_27123.html"));
// print_r($obj->GetAudio("201_3751_1", "1", "http://www.pingshu8.com/play_161404.html"));
?>
