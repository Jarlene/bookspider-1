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

//	Action();
	Action2();
//	Action3();
//	Action4();

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function GetTopPingshu()
	{
		$books = array();

//		$uri = "http://www.pingshu8.com/top/pingshu.htm";
		$uri = "http://www.pingshu8.com/top/yousheng.htm";
//		$uri = "http://www.pingshu8.com/top/xiangsheng.htm";
//		$uri = "http://www.pingshu8.com/top/zongyi.htm";
//		$uri = "http://www.pingshu8.com/Special/Msp_218.Htm";
		$html = http_proxy_get($uri, "luckyzz@163.com", 20);
		//$html = http_get($uri);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $books;

		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='tab3']/a | //div[@class='tab33']/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$book = $element->nodeValue;

			$bookid = basename($href);
			$n = strpos($bookid, '.');
			$books[substr($bookid, 4, $n-4)] = $book;
		}

		return $books;
	}

	function DownloadBook($bookid, $bookname, $book)
	{
		global $db;
		global $client;

		// $book = $site->GetBookInfo($bookid);
		$dbchapters = $db->get_chapters(CPingShu8::$siteid, $bookid);
		if($book["count"] == count($dbchapters) && count($dbchapters) > 0){
			print_r("book has download.\n");
			return 0;
		}

		$site = new CPingShu8();
		//$site->useDelegate = 0;
		$chapters = $site->WebGetChapters($bookid);

		if(strlen($book["icon"]) < 1 && strlen($chapters["icon"]) > 1){
			$book = $chapters;
			print_r("DB book update2($bookid, $bookname)\n");
			if(0 != $db->update_book(CPingShu8::$siteid, $bookid, "", $bookname, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"]))
				print_r("update book2($bookid) error: " . $db->get_error . "\n");
		}

		$chapters = $chapters["chapter"];
		print_r("db chapters: " . count($dbchapters) . " chapters: " . count($chapters) . "\n");
		foreach($chapters as $chapter){
			$chapterid = $chapter["uri"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				$db->add_chapter(CPingShu8::$siteid, $bookid, $chapterid, $name, "");
			} else {
				$dbchapter = $dbchapters[$chapterid];
				if(strlen($dbchapter["uri"]) > 0)
					continue;
			}

			print_r("Add task($bookid, $chapterid)\n");
			$workload = sprintf("%s,%d", $bookid, $chapterid);
			$client->doBackground('DownloadPingshu8', $workload);
		}
	}

	function Action()
	{
		global $db;
		//$site = new CPingShu8();
		//$books = $site->GetTopPingshu();
 		$books = GetTopPingshu();
		$n = count($books);
		print_r("Get Books: " . count($books) . "\n");

		$i = 0;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($books as $id => $name)
		{
			$i++;
			//if($i < 900)
			//	 continue;
			
			// if($i > 300)
				// break;

			// if ($id == "203_1517_1")
				// continue;

			$site = new CPingShu8();
			//$site->useDelegate = 0;
			//sleep(10);
			$book = $site->GetBookInfo($id);

			if(!array_key_exists($id, $dbbooks))
			{
				print_r("DB add book($id, $name)\n");
				//$db->add_book($id, "", $name, "", "", "", "");
				if(0 != $db->add_book(CPingShu8::$siteid, $id, "", $name, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"]))
					print_r("add book($id) error: " . $db->get_error . "\n");
			} else {
				$dbbook = $dbbooks[$id];
				if(strlen($book["icon"]) > 1 && (strlen($dbbook["icon"]) < 1 || strlen($dbbook["summary"]) < 1)){
					print_r("DB book update($id, $name)\n");
					if(0 != $db->update_book(CPingShu8::$siteid, $id, "", $name, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"]))
						print_r("update book($id) error: " . $db->get_error . "\n");
				}
			}

			print_r("Download([$i]$id)\n");
			DownloadBook($id, $name, $book);
		}
	}

	function Action2()
	{
		global $db;
		global $client;

		$i = 0;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters(CPingShu8::$siteid, $bookid);
			if(count($dbchapters) > 0)
				continue;

			$i++;	
			print_r("Download([$i]$bookid)\n");
			$site = new CPingShu8();
			$chapters = $site->WebGetChapters($bookid);
			$chapters = $chapters["chapter"];
	
			foreach($chapters as $chapter){
				$chapterid = $chapter["uri"];
				$name = $chapter["name"];

				if(0 != $db->add_chapter(CPingShu8::$siteid, $bookid, $chapterid, $name, ""))
					print_r("DB add chapter($bookid, $chapterid, $name) failed\n");

				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadPingshu8', $workload);
			}
		}
	}
	
	function Action3()
	{
		global $db;
		global $client;

		$i = 0;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters(CPingShu8::$siteid, $bookid);
			foreach($dbchapters as $chapterid => $dbchapter)
			{
				if(strlen($dbchapter["uri"]) > 1)
					continue;

				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadPingshu8', $workload);
			}
		}
	}

	function Action4()
	{
		global $db;

 		$books = GetTopPingshu();
		$n = count($books);
		print_r("Get Books: " . count($books) . "\n");

		$i = 0;
		$dbbooks = $db->get_books(CPingShu8::$siteid);
		foreach($books as $id => $name)
		{
			$i++;
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("[$i/$n]DB add book($id, $name)\n");
				$db->add_book(CPingShu8::$siteid, $id, "", $name, "", "", "", "");
			} 
		}
	}
?>
