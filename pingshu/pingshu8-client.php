<?php
	require_once("php/util.inc");
	require_once("php/dom.inc");
	require_once("php/http.inc");
	require_once("http-proxy.php");
	require_once("php/http-multiple.inc");
	require_once("http-multiple-proxy.php");
	require("pingshu8.php");

	$client = new GearmanClient();
	$client->addServer("115.28.54.237", 4730);	
	$db = new DBPingShu("115.28.54.237");

	$useDelegate = 0;
	
	$update_mode = TRUE; // 只找没有的书
	//Action1($update_mode);
	//Action2($update_mode);
	//Action3();
	if($update_mode)// 每天调用更新一次
	{
		Action4($update_mode); 
		Action3();
	}

	function Action1($update_mode)
	{
		$books = GetBooks($update_mode);
		$n = count($books);
		print_r("Get Books: " . count($books) . "\n");

		$i = 0;
		global $db;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($books as $id => $name)
		{
			$i++;
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("[$i]DB add book($id, $name)\n");
				if(0 != $db->add_book(CPingShu8::$siteid, $id, "", $name, "", "", "", "")){
					print_r("add book($id) error: " . $db->get_error() . "\n");
					die();
				}
			}
		}
	}

	function Action2($update_mode)
	{
		global $db;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		print_r("db-book count: " . count($dbbooks) . "\n");

		$i = 0;	
		foreach($dbbooks as $bookid => $dbbook)
		{
			$i++;
			$name = $dbbook["name"];
			print_r("AddChapter([$i]$bookid - $name)\n");
			if(0 != AddChapter($bookid, $dbbook, $update_mode))
				sleep(20);
		}
	}

	function Action3()
	{
		$i = 0;
		global $db;
		global $client;

		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters(CPingShu8::$siteid, $bookid);
			foreach($dbchapters as $chapterid => $dbchapter)
			{
				$uri = $dbchapter["uri"];
				$uri2 = $dbchapter["uri2"];
				if(strlen($uri2) > 1 || 0==strncmp("175.195.249.184", $uri, 10))
				//if(strlen($uri) > 1 || strlen($uri2) > 1)
					continue;

				++$i;
				print_r("[$i]Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadPingshu8', $workload, $workload);
			}
		}
	}
	
	function Action4($update_mode)
	{
		$books = GetBooks($update_mode);
		$n = count($books);
		print_r("Get Books: " . count($books) . "\n");
	
		$i = 0;
		global $db;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($books as $id => $name)
		{
			$i++;
			print_r("$id = $name \n");
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("[$i]DB add book($id, $name)\n");
				if(0 != $db->add_book(CPingShu8::$siteid, $id, "", $name, "", "", "", "")){
					print_r("add book($id) error: " . $db->get_error() . "\n");
					die();
				}

				$dbbooks[$id] = array("icon" => "");
			}

			if(0 != AddChapter($id, $dbbooks[$id], $update_mode))
				sleep(20);
		}
	}

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function GetBooks($update_mode)
	{
		global $useDelegate;
		$books = array();

		$urls = array();
		if($update_mode){
			$urls[] = "http://www.pingshu8.com/music/newzj.htm";
		} else {
			$urls[] = "http://www.pingshu8.com/top/pingshu.htm";
			$urls[] = "http://www.pingshu8.com/top/yousheng.htm";
			$urls[]= "http://www.pingshu8.com/top/xiangsheng.htm";
			$urls[]= "http://www.pingshu8.com/top/zongyi.htm";
			$urls[]= "http://www.pingshu8.com/Special/Msp_218.Htm";
		}

		foreach($urls as $uri)
		{
			if ($useDelegate == 1)
				$html = http_proxy_get($uri, "luckyzz@163.com", 20);
			else
				$html = http_get($uri);
			
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			if(strlen($html) < 1){
				print_r("Load failed: $uri\n");
				continue;
			}

			$xpath = new XPath($html);
			
			if(0 == strcmp($uri, 'http://www.pingshu8.com/music/newzj.htm')){
				$elements = $xpath->query("//div[@class='tab3']/ul/li/a[2]");
				foreach ($elements as $element){
					$href = $element->getattribute('href');
					$book = $xpath->get_value("span", $element);

					$bookid = basename($href);
					$n = strpos($bookid, '.');
					$books[substr($bookid, 4, $n-4)] = $book;
				}
			} else {
				$elements = $xpath->query("//div[@class='tab3']/a | //div[@class='tab33']/a");
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					$bookid = basename($href);
					$n = strpos($bookid, '.');
					$books[substr($bookid, 4, $n-4)] = $book;
				}
			}
		}

		return $books;
	}

	function AddChapter($bookid, $dbbook, $update_mode)
	{
		global $db;
		global $client;

		$dbchapters = $db->get_chapters(CPingShu8::$siteid, $bookid);
		if(1 != $update_mode && count($dbchapters) > 0){
			print_r("book has download.\n");
			return 0;
		}

		$book = __WebGetChapters($bookid, count($dbchapters));
		if(strlen($dbbook["icon"]) < 1 && strlen($book["icon"]) > 1){
			$bookname = $dbbook["name"];
			print_r("DB book update($bookid, $bookname)\n");
			if(0 != $db->update_book(CPingShu8::$siteid, $bookid, "", $bookname, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"])){
				print_r("update book2($bookid) error: " . $db->get_error() . "\n");
				die();
			}
		}

		$chapters = $book["chapter"];
		print_r("db chapters: " . count($dbchapters) . " chapters: " . count($chapters) . "\n");
		foreach($chapters as $chapter){
			$chapterid = $chapter["uri"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				if(0 != $db->add_chapter(CPingShu8::$siteid, $bookid, $chapterid, $name, "")){
					print_r("add_chapter($bookid, $chapterid) error: " . $db->get_error() . "\n");
					die();
				}

				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadPingshu8', $workload, $workload);
			}
		}

		return 1;
	}

	//----------------------------------------------------------------------------
	// Website
	//----------------------------------------------------------------------------	
	function __WebGetChapters($bookid, $dbChapterCount)
	{
		global $useDelegate;
		
		$book = __WebGetBookInfo($bookid);
		
		$updateCount = $book["count"] - $dbChapterCount;
		
		if ($updateCount%10)
			$addPage = 1;
		else 
			$addPage = 0;
		$updatePage = $updateCount/10 + $addPage;

		list($v1, $v2, $v3) = explode("_", $bookid);

		$urls = array();
		$page = $book["page"];
		for($i = ($page-$updatePage)+1; $i < $page; $i++){
			$urls[] = sprintf("http://www.pingshu8.com/MusicList/mmc_%d_%d_%d.htm", $v1, $v2, $i+1);
		}

		if(count($urls) > 0){
			$result = array();
			
			if ($useDelegate == 1)
				$http = new HttpMultipleProxy("proxy.cfg");
			else
				$http = new HttpMultiple();

			$r = $http->get($urls, '_OnReadChapter', &$result, 20);

			if(count($result) != count($urls)){
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

	function __WebGetBookInfo($bookid)
	{
		global $useDelegate;
		
		$uri = "http://www.pingshu8.com/MusicList/mmc_$bookid.htm";
		$referer = "Referer: http://www.pingshu8.com/top/xiangsheng.htm";
		
		if ($useDelegate == 1)
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
		$data["chapter"] = __ParseChapter($html);
		$data["catalog"] = $xpath->get_value("//div[@class='t1']/div/a[2]");
		$data["subcatalog"] = $xpath->get_value("//div[@class='t1']/div/a[3]");
		return $data;
	}

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

		$param[$i] = __ParseChapter($body);
		return 0;
	}

	function __ParseChapter($html)
	{
		$chapters = array();

		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $chapters;

		$xpath = new XPath($html);
		$elements = $xpath->query("//li[@class='a1']/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$chapter = $element->nodeValue;

			if(strlen($href) > 0 && strlen($chapter) > 0)
			{
				list($play, $chapterid) = explode("_", basename($href, ".html"));
				$chapters[] = array("name" => $chapter, "uri" => $chapterid);
			}
		}

		return $chapters;
	}
?>
