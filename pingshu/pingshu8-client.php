<?php
	require_once("php/dom.inc");
	require_once("phttp.php");
	require_once("db-pingshu.inc");

	$siteid = 1;
	$db = new DBPingShu("115.28.54.237");
	$http = new PHttp();

	$update_mode = TRUE;
	if($update_mode){
		Action1($siteid, True); 
	} else {
		Action1($siteid, False);
		Action2($siteid);
		Action3($siteid);
	}

	function Action1($siteid, $update_mode)
	{
		global $db;

		$books = GetBooks($update_mode);
		$n = count($books);
		print_r("Get Books: " . count($books) . "\n");

		$i = 0;
		$dbbooks = $db->get_books($siteid);
		foreach($books as $id => $name)
		{
			$i++;
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("[$i]DB add book($id, $name)\n");
				if(0 != $db->add_book($siteid, $id, "", $name, "", "", "", "")){
					print_r("add book($id) error: " . $db->get_error() . "\n");
					die();
				}

				$dbbooks[$id] = array("icon" => ""); // for AddChapter
			}

			if($update_mode){
				print_r("AddChapter([$i]$bookid - $name)\n");
				if(0 != AddChapter($siteid, $id, $dbbooks[$id], $update_mode))
					sleep(20);
			}
		}
	}

	function Action2($siteid)
	{
		global $db;
		$dbbooks = $db->get_books($siteid);
		print_r("db-book count: " . count($dbbooks) . "\n");

		$i = 0;
		foreach($dbbooks as $bookid => $dbbook)
		{
			$i++;
			$name = $dbbook["name"];
			print_r("AddChapter([$i]$bookid - $name)\n");
			if(0 != AddChapter($siteid, $bookid, $dbbook, False))
				sleep(20);
		}
	}

	function Action3($siteid)
	{
		global $db;

		$client = new GearmanClient();
		$client->addServer("115.28.54.237", 4730);

		$i = 0;
		$dbbooks = $db->get_books($siteid);
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters($siteid, $bookid);
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

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function GetBooks($update_mode)
	{
		global $http;
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
			$html = $http->get($uri, "luckyzz@163.com");
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

	function AddChapter($siteid, $bookid, $dbbook, $update_mode)
	{
		global $db;

		$dbchapters = $db->get_chapters($siteid, $bookid);
		if(!$update_mode && count($dbchapters) > 0){
			print_r("book has download.\n");
			return 0;
		}

		// load book info
		$book = __WebGetBookInfo($bookid);
		if(False === $book){
			print_r("AddChapter($siteid, $bookid) failed.\n");
			return 1;
		}

		if(strlen($dbbook["icon"]) < 1 && strlen($book["icon"]) > 1){
			$bookname = $dbbook["name"];
			print_r("DB book update($bookid, $bookname)\n");
			if(0 != $db->update_book($siteid, $bookid, "", $bookname, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"])){
				print_r("update book2($bookid) error: " . $db->get_error() . "\n");
				die();
			}
		}

		if($book["count"] == count($dbchapters)){
			print_r("book has download.\n");
			return 1;	
		}

		// load chapters
		$chapters = __WebGetChapters($bookid, $book, count($dbchapters));
		print_r("db chapters: " . count($dbchapters) . " chapters: " . count($chapters) . "\n");
		foreach($chapters as $chapter){
			$chapterid = $chapter["uri"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				if(0 != $db->add_chapter($siteid, $bookid, $chapterid, $name, "")){
					print_r("add_chapter($bookid, $chapterid) error: " . $db->get_error() . "\n");
					die();
				}
			}
		}

		return 1;
	}

	//----------------------------------------------------------------------------
	// Website
	//----------------------------------------------------------------------------	
	function __WebGetChapters($bookid, $book, $dbChapterCount)
	{
		global $http;
		list($v1, $v2, $v3) = explode("_", $bookid);

		$page = $book["page"];
		// $n = $dbChapterCount/10;
		// for($i = $n>0 ? $n : 1; $i < $page; $i++){
		for($i = 1; $i < $page; $i++){
			$uri = sprintf("http://www.pingshu8.com/MusicList/mmc_%d_%d_%d.htm", $v1, $v2, $i+1);
			$html = $http->get($uri, "luckyzz@163.com", array("referer" => $referer));
			if(strlen($html) < 1){
				print_r("__WebGetChapters($uri) failed.\n");
				return False;
			}

			$chapters = __ParseChapter($html);
			foreach($chapters as $chapter){
				$book["chapter"][] = $chapter;
			}
		}

		return $book["chapter"];
	}

	function __WebGetBookInfo($bookid)
	{
		global $http;
		
		$uri = "http://www.pingshu8.com/MusicList/mmc_$bookid.htm";
		$referer = "Referer: http://www.pingshu8.com/top/xiangsheng.htm";
		$html = $http->get($uri, "luckyzz@163.com", array("referer" => $referer));
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return False;

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

		$data = array();
		list($count) = sscanf($value, " 共有%d集");
		$data["info"] = $xpath->get_value("//div[@class='c']/div");
		$data["page"] = $selects->length;
		$data["count"] = $count;
		$data["chapter"] = __ParseChapter($html);
		$data["catalog"] = $xpath->get_value("//div[@class='t1']/div/a[2]");
		$data["subcatalog"] = $xpath->get_value("//div[@class='t1']/div/a[3]");
		return $data;
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
