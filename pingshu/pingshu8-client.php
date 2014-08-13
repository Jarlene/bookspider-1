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

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function GetTopPingshu()
	{
		$books = array();

//		$uri = "http://www.pingshu8.com/top/pingshu.htm";
//		$uri = "http://www.pingshu8.com/top/yousheng.htm";
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

		// $book = $obj->GetBookInfo($bookid);
		$dbchapters = $db->get_chapters($bookid);
		if($book["count"] == count($dbchapters) && count($dbchapters) > 0){
			print_r("book has download.\n");
			return 0;
		}

		$obj = new CPingShu8();
		//$obj->useDelegate = 0;
		$chapters = $obj->WebGetChapters($bookid);

		if(strlen($book["icon"]) < 1 && strlen($chapters["icon"]) > 1){
			$book = $chapters;
			print_r("DB book update2($bookid, $bookname)\n");
			if(0 != $db->update_book($bookid, "", $bookname, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"]))
				print_r("update book2($bookid) error: " . $db->get_error . "\n");
		}

		$chapters = $chapters["chapter"];
		print_r("db chapters: " . count($dbchapters) . " chapters: " . count($chapters) . "\n");
		foreach($chapters as $chapter){
			$chapterid = $chapter["uri"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				$db->add_chapter($bookid, $chapterid, $name, "");
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
		//$obj = new CPingShu8();
		//$books = $obj->GetTopPingshu();
 		$books = GetTopPingshu();
		$n = count($books);
		print_r("Get Books: " . count($books) . "\n");

		$i = 0;
		$dbbooks = $db->get_books();
		foreach($books as $id => $name)
		{
			$i++;
			//if($i < 900)
			//	 continue;
			
			// if($i > 300)
				// break;

			// if ($id == "203_1517_1")
				// continue;

			$obj = new CPingShu8();
			//$obj->useDelegate = 0;
			//sleep(10);
			$book = $obj->GetBookInfo($id);

			if(!array_key_exists($id, $dbbooks))
			{
				print_r("DB add book($id, $name)\n");
				//$db->add_book($id, "", $name, "", "", "", "");
				if(0 != $db->add_book($id, "", $name, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"]))
					print_r("add book($id) error: " . $db->get_error . "\n");
			} else {
				$dbbook = $dbbooks[$id];
				if(strlen($book["icon"]) > 1 && (strlen($dbbook["icon"]) < 1 || strlen($dbbook["summary"]) < 1)){
					print_r("DB book update($id, $name)\n");
					if(0 != $db->update_book($id, "", $name, $book["icon"], $book["info"], $book["catalog"], $book["subcatalog"]))
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
		$dbbooks = $db->get_books();
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters($bookid);
			if(count($dbchapters) > 0)
				continue;

			$i++;	
			print_r("Download([$i]$bookid)\n");
			$obj = new CPingShu8();
			$chapters = $obj->WebGetChapters($bookid);
			$chapters = $chapters["chapter"];
	
			foreach($chapters as $chapter){
				$chapterid = $chapter["uri"];
				$name = $chapter["name"];

				if(0 != $db->add_chapter($bookid, $chapterid, $name, ""))
					print_r("DB add chapter($bookid, $chapterid, $name) failed\n");

				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadPingshu8', $workload);
			}
		}
	}
?>
