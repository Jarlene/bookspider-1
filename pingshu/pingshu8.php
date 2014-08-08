<?php	
// require("php/db.inc");

// $db = dbopen("pingshu8");
// if($db->connect_errno){
	// echo "mysql error " . $db->connect->error;
// }
	
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
	function GetName()
	{
		return "pingshu8";
	}
	
	function GetRedirect()
	{
		return 0;
		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);
		$mdbkey = "ts-server-" . $this->GetName() . "-audio-mode";
		$mode = $mdb->get($mdbkey);
		
		if(!$mode)
		{
			$mode = $this->getModePingshu8();
			$mdb->set($mdbkey, $mode, 2 * 60); // update per 0.5-hour*24
		}
		
//  		print_r("mode". $mode);
		
		return $mode;
	}
	
	
	function getModePingshu8()
	{
		$index = rand(1,5);
		
		if ($index == 1)
			$uri = "http://www.pingshu8.com/play_182620.html";
		else if ($index == 2)
			$uri = "http://www.pingshu8.com/play_161406.html";
		else if ($index == 3)
			$uri = "http://www.pingshu8.com/play_143995.html";
		else if ($index == 4)
			$uri = "http://www.pingshu8.com/play_20148.html";
		else if ($index == 5)
			$uri = "http://www.pingshu8.com/play_20152.html";
		else 
			$uri = "http://www.pingshu8.com/play_182620.html";
		
		
		if ($this->useDelegate == 1)
			$html = http_proxy_get($uri,"pingshu8.com", 10, "proxy.cfg");
		else
			$html = http_get($uri, 10, "");
		
		if(!preg_match('/encodeURI\(\"(.+)\"\)/', $html, $matches))
		{
			return 2;
		}
		
		if(2 != count($matches))
		{
			return 2;
		}
		
		$uri = $matches[1];
		if (strlen($uri) > 15)
			return 1;
		else 
			return 2;
	}

	
	function getMiddlePath($bookid, $chapter, $uri)
	{
		
		
		$headers = array();
		$data = array();
		
		$headers["Referer"] = $uri;

		$headers["User-Agent"] = "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)";
		$headers["User-Agent"] = getRandomUserAgent();
		
		$path = $this->getLaestPath($uri);
		
		$uri = str_replace("play_", $path, $uri);
		
		$data["url"] = $uri;
		$data["headers"] = $headers;
		
		return $data;
	}
	
	function getLaestPath($uri)
	{
		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);
		$mdbkey = "ts-server-" . $this->GetName() . "-audio-path";
		$rawuri = $mdb->get($mdbkey);
		
		$path = "";
		if($rawuri)
		{
			$path = $rawuri;
		}
		else 
		{
			$path = $this->getPathFromPingshu8($uri);
			
			if (strlen($path) > 0)
				$mdb->set($mdbkey, $path, 5 * 60); // update per 0.5-hour*24
		}
		
		
		
		return $path;
	}
	
	function getPathFromPingshu8($uri)
	{
		
		
// 		$headers = array();
// 		$headers[] = "User-Agent: Mozilla/4.0 (compatible; MSIE 8.5; Windows NT 6.2; Win64; x64; Trident/4.0)";
// 		$headers[] = "Referer: " . $uri;
		
		if ($this->useDelegate == 1)
			$html = http_proxy_get("http://www.pingshu8.com/Play_Flash/js/play.js?0.00.90","pingshu8.com", 10, "proxy.cfg", array("Referer: " . $uri));
		else
			$html = http_get("http://www.pingshu8.com/Play_Flash/js/play.js?0.00.90", 10, "", array("Referer: " . $uri));

		
// 		print_r($html . "kkjdfjkf");
// 		return "";
		
		$js = getGapString($html, "js/", ".js");
		
		
		
 		$js = "http://www.pingshu8.com/Play_Flash/js/" . $js . ".js?0.00.90";
		
 		

// 		$js = "http://www.pingshu8.com/Play_Flash/js/b.js?1.00.18";
		
		if ($this->useDelegate == 1)
			$html = http_proxy_get($js,"urlpath", 10, "proxy.cfg", array("Referer: " . $uri));
		else
			$html = http_get($js, 10, "", array("Referer: " . $uri));
		$forder = getGapString($html, "url: \"", "\" + urlpath");
		
		
		
		if (strlen($forder)<=0)
			$forder = getGapString($html, "url|", "|type");
		
		
		return $forder;
		
		// 验证获得path的有效性
		$referer = $uri;
		$retPath = str_replace("play_", $forder, $uri);
		
		
		
		
		if ($this->useDelegate == 1)
			$html = http_proxy_post($retPath, "", ":8000", 10, "proxy.cfg", array("Referer: " . $referer));
		else
			$html = http_get($retPath, 10, "", array("Referer: " . $referer));
		
// 		file_put_contents ("a.html", "getPathFromPingshu8:". $js . "*" . $forder . "*" . $retPath . "*" . $html);
		
		$obj = json_decode($html);
		$uri2 = $obj->{"urlpath"};
		
// 		file_put_contents ("a.html", "getPathFromPingshu8:". $js . "*" . $forder . "*" . $retPath . "*" . $uri2);
		
		
		if (strlen($uri2) > 0)
			return $forder;
		else 
			return "";
	}
	
	
	function getLaestReplaceValue($uri)
	{
// 		return "9";
		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);
		$mdbkey = "ts-server-" . $this->GetName() . "-audio-replacevalue";
		
		
		$rawuri = $mdb->get($mdbkey);
	
// 		file_put_contents ("a.html", "value:". $rawuri);
		
		$path = array();
		if($rawuri)
		{
			$path = json_decode($rawuri, True);
		}
		else
		{
			$path = $this->getReplaceValueFromPingshu8($uri);
				
			if (count($path) > 0)
				$mdb->set($mdbkey,  json_encode($path), 2 * 60); // update per 0.5-hour*24
		}
		
//  		file_put_contents ("a.html", "value:". $path[0] . $path[1]);
	
		return $path;
	}
	function getReplaceValueFromPingshu8($uri)
	{
	
// 		if ($this->useDelegate == 1)
// 			$html = http_proxy_get("http://www.pingshu8.com/Play_Flash/js/play.js?0.00.90","pingshu8.com", 10, "proxy.cfg", array("Referer: " . $uri));
// 		else
// 			$html = http_get("http://www.pingshu8.com/Play_Flash/js/play.js?0.00.90", 10, "", array("Referer: " . $uri));
	
		
// 		$js = getGapString($html, "js/", ".js");
// 		$js = "http://www.pingshu8.com/Play_Flash/js/" . $js . ".js?0.00.90";
	
		
		$forder = array();
		$js = "http://www.pingshu8.com/Play_Flash/js/b.js?0.00.90";
	
		if ($this->useDelegate == 1)
			$html = http_proxy_get($js,"urlpath", 10, "proxy.cfg", array("Referer: " . $uri));
		else
			$html = http_get($js, 10, "", array("Referer: " . $uri));
		$forder[] = getGapString($html, "RegExp(\"\\\\", "\",");
		$forder[] = getGapString($html, "(regS,\"", "\");");
		
	
		return $forder;
	
// 		// 验证获得path的有效性
// 		$referer = $uri;
// 		$retPath = str_replace("play_", $forder, $uri);
// 		if ($this->useDelegate == 1)
// 			$html = http_proxy_post($retPath, "", ":8000", 10, "proxy.cfg", array("Referer: " . $referer));
// 		else
// 			$html = http_get($retPath, 10, "", array("Referer: " . $referer));
	
// 		$obj = json_decode($html);
// 		$uri2 = $obj->{"urlpath"};
// 		if (strlen($uri2) > 0)
// 			return $forder;
// 		else
// 			return "";
	}
	
	
	function GetAudio3($bookid, $chapter, $uri)
	{
		// 方式2，path96
		if ($this->GetRedirect() == 2)
		{
			return $this->GetAudio_2($bookid, $chapter, $uri);
		}
		
		// 方式1， encodeURI(\"http://play0.pingshu8.com:8000/0/ys/外婆来讲鬼故事(34集)/外婆来讲鬼故事_01.flv?1320@123abcd8@123abcd4863x1407287561x13736537235-
		if (0 != $this->redirect)
		{
			$html = $uri;
		}
		else
		{
	
			if ($this->useDelegate == 1)
			$html = http_proxy_get($uri,"pingshu8.com", 10, "proxy.cfg");
			else
			$html = http_get($uri, 10, "");
		}

//		$html = "var urlpath = encodeURI(\"http://play0.pingshu8.com:8000/0/ys/外婆来讲鬼故事(34集)/外婆来讲鬼故事_01.flv?1320@123abcd8@123abcd4863x1407287561x13736537235-6fc@123abcd5720dbc7fcd0b441@123abcd0@123abcdad4e6cf2@123abcd\");</script>";

		if(!preg_match('/encodeURI\(\"(.+)\"\)/', $html, $matches))
		{
			return "no preg_match";
		}
		
		if(2 != count($matches))
		{
			return "no count";
		}
		
		$uri = $matches[1];
		
		$value = array();
		$value = $this->getLaestReplaceValue("http://www.pingshu8.com/play_182620.html");
		
// 		return "value:" . $value;
		
// 		print_r($value);
		
// 		file_put_contents ("a.html", $value);
		if (count($value) > 1)
			$uri = str_replace($value[0], $value[1], $uri);
		
		$uri = str_replace(".flv", ".mp3", $uri);
	
		return  encodeUrlStr($uri);
	}
	
	function GetAudio_2($bookid, $chapter, $uri)
	{
		if (0 != $this->redirect)
		{
			$obj = json_decode($uri);
			$uri = $obj->{"urlpath"};
			$uri = str_replace(".flv", ".mp3", $uri);
			$uri = str_replace("7j2io9", "8", $uri);
			
			return  $uri;
		}
		
		return  $uri;
// 		$this->__GetAudio($bookid, $uri);

		
		
// 		return $this->__EncodeAudioURI($rawuri);
	}

	function GetAudio($bookid, $chapter, $uri)
	{
		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);
		$mdbkey = "ts-server-" . $this->GetName() . "-audio-$bookid-$chapter";
		$rawuri = $mdb->get($mdbkey);
		if(!$rawuri){
			$rawuri = $this->__GetAudio($bookid, $uri);
			if(strlen($rawuri) > 0){
				//$rawuri = substr($uri, 0, strpos($uri, '?'));
				$mdb->set($mdbkey, $rawuri, 60 * 60); // update per 1-hour
			} else {
				return "";
			}
		}
		
		$chapterid = basename($uri, ".html");
		$n = strpos($chapterid, '_');
		$chapterid = substr($chapterid, $n+1);
		return $this->__EncodeAudioURI($bookid, $chapterid, $rawuri);
	}

	function __EncodeAudioURI($bookid, $chapter, $uri)
	{
		$n = strrpos($uri, '?');
		$suffix = substr($uri, $n+1);
		$uri = substr($uri, 0, $n);

		//DBSetAudioFile($bookid, $chapter, $uri);

		list($v1, $v0, $v2) = sscanf($suffix, "%ux%ux%u-");

		// $uri = str_replace("@123abcd", "9", $uri);
		// $uri = str_replace(".flv", ".mp3", $uri);
		// $uri = str_replace("play0.", "p0a1.", $uri);
		// $uri = str_replace("play1.", "p1a1.", $uri);

		$ip = $this->__ip();
		$ip = str_replace(".", "0", $ip);

		$t = time();
		$postfix = sprintf("?%ux%ux%u-6618f00ff155173c7dddb190142ace21", $t+$ip, $t, $t+$v2-$v1+$ip);

		$uri = $uri . $postfix;
		// //return iconv("gb18030", "UTF-8", $uri);

		// $ip = $this->__ip();
		// $ip = str_replace(".", "0", $ip);

		// $t = time();
		// $postfix = sprintf("?%ux%ux%u-6618f00ff155173c7dddb190142ace21", $t+$ip, $t, $t+526642372+$ip);

		// $uri = $rawuri . $postfix;
		// $uri = str_replace("pl0.", "p0a1.", $uri);
		// $uri = str_replace("pl1.", "p1a1.", $uri);
		return $uri;
	}

	function __GetEncStr($bookid, $uri)
	{
		$referer = "Referer: http://www.pingshu8.com/MusicList/mmc_" . $bookid . ".htm";
		$html = http_proxy_get($uri, "luckyzz@163.com", 10, "proxy.cfg", array($referer));
		if(!preg_match('/var encrystr =\"(.+)\"/', $html, $matches)){
			return "";
		}
		
		if(2 == count($matches)){
			return $matches[1];
		}
		return "";
	}

	function __GetAudio($bookid, $uri)
	{
		$encrystr = $this->__GetEncStr($bookid, $uri);

		$bookid = basename($uri);
		$n = strpos($bookid, '_');
		$id = substr($bookid, $n+1);
		$uri = "http://www.pingshu8.com/path_" . $id;
		$referer = "Referer: http://www.pingshu8.com/play_" . $id;
		$postdata = "encrystr=$encrystr&urlpath=" . basename($id, ".html");
		//$html = http_proxy_post($uri, "", "luckyzz@163.com", 10, "proxy.cfg", array("Referer: http://www.pingshu8.com/play_161404.html"));
		$html = http_proxy_post($uri, $postdata, ":8000", 10, "proxy.cfg", array($referer));
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
		// $n = strrpos($uri, '?');
		// $uri = substr($uri, 0, $n);
		// $suffix = substr($uri, $n+1);

		// //$uri = str_replace("@123abcd", "9", $uri);
		// $uri = str_replace(".flv", ".mp3", $uri);
		// $uri = str_replace("play0.", "p0a1.", $uri);
		// $uri = str_replace("play1.", "p1a1.", $uri);

		// $ip = $this->__ip();
		// $ip = str_replace(".", "0", $ip);

		// $t = time();
		// $postfix = sprintf("?%ux%ux%u-6618f00ff155173c7dddb190142ace21", $t+$ip, $t, $t+526642372+$ip);
		
		// $uri = $uri . $postfix;
		// //return iconv("gb18030", "UTF-8", $uri);
		return $uri;
	}

	function DBSetAudioFile($bookid, $chapterid, $uri)
	{
		global $db;
		//$sql = sprintf('update audio set uri="%s" where chapterid=%d', $uri, $chapterid);
		$sql = sprintf('insert into audio (bookid, chapterid, uri) values ("%s", %d, "%s")', $bookid, $chapterid, $uri);
		if(!$db->query($sql)){
			print_r("DB set uri failed: " . $db->error);
		}
		return $db->error;
	}

	function DBGetAudioFile($bookid, $chapterid)
	{
		global $db;
		$sql = sprintf('select uri where chapterid=%d', $uri, $chapterid);
		$res = $db->query($sql);
		if(FALSE === $res){
			return "";
		}

		$uri = "";
		while($row = $res->fetch_assoc())
		{
			$uri = $row["uri"];
		}
		$res->free();
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

	function __ParseChapters($html)
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

	function __GetSubcatalog($uri)
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
	
	function __SearchAuthor($keyword)
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

	function __SearchBook($keyword)
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
	
	function __ip()
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

// print_r($obj->getReplaceValueFromPingshu8("http://www.pingshu8.com/play_182620.html")); sleep(2);
// print_r($obj->getPathFromPingshu8("http://www.pingshu8.com/play_182620.html")); sleep(2);
// print_r($obj->GetCatalog()); sleep(2);
// print_r($obj->GetBooks("http://www.pingshu8.com/Special/Msp_7.Htm")); sleep(2);
// print_r($obj->GetChapters('7_208_1')); sleep(2);
// print_r($obj->GetAudio('7_208_1', '1', "http://www.pingshu8.com/play_19632.html")); sleep(2);
// print_r($obj->Search("单田芳")); sleep(2);
// print_r($obj->__GetAudio("http://www.pingshu8.com/play_27123.html"));
// print_r($obj->GetAudio("201_3751_1", "1", "http://www.pingshu8.com/play_161404.html"));
?>
