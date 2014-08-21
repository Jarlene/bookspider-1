<?php
	require_once("php/dom.inc");
	require_once("php/http.inc");
	require_once("http-proxy.php");
	require_once("db-pingshu.inc");
	require_once("ysts8.php");

	$db = new DBPingShu("115.28.54.237");

	Action1();
	Action2(1);
	Action3();

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function _GetBooks($uri)
	{
		$books = array();

		$html = http_proxy_get($uri, "Ysjs/bot.js", 10);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $books;

		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='pingshu_ysts8_i']/ul/li/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$book = $element->nodeValue;

			$bookid = basename($href, ".html");
			$books[substr($bookid, 2)] = $book;
		}

		return $books;
	}

	function GetHot()
	{
		return _GetBooks("http://www.ysts8.com/index_hot.html");
	}

	function GetUpdate()
	{
		return _GetBooks("http://www.ysts8.com/index_tim.html");
	}

	function GetAll()
	{
		$books = array();
		$site = new CYSTS8();
		$catalogs = $site->GetCatalog();
		$catalogs = $catalogs["小说"];
		foreach ($catalogs as $name => $value) {
			$catalog = $site->GetBooks($value);
			$books = array_merge($books, $catalog["book"]);
		}
		return $books;
	}

	function AddBook($books)
	{
		global $db;

 		$i = 0;
		$dbbooks = $db->get_books(CYSTS8::$siteid);
		foreach($books as $id => $name)
		{
			$i++;
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("[$i]DB add book($id, $name)\n");
				$db->add_book(CYSTS8::$siteid, $id, "", $name, "", "", "", "");
			} 
		}
	}

	function AddChapter($bookid, $bookname, $fast_mode)
	{
		global $db;
		global $client;

		$dbchapters = $db->get_chapters(CYSTS8::$siteid, $bookid);
		if(1 == $fast_mode && count($dbchapters) > 0){
			// don't in update mode
			print_r("book has download.\n");
			return 0;
		}

		$site = new CYSTS8();
		$result = $site->GetChapters($bookid);
		$chapters = $result["chapter"];
		if(count($chapters) < 1){
			print_r("GetChapters($bookid) error.\n");
			return -1;
		} else if(count($dbchapters) == count($chapters)){
			print_r("book has download.\n");
			return 0;
		}

		if(count($dbchapters)<1 && strlen($result["info"]) > 1){
			print_r("DB book update($bookid, $bookname)\n");
			if(0 != $db->update_book(CYSTS8::$siteid, $bookid, "", $bookname, $result["icon"], $result["info"], $result["catalog"], $result["subcatalog"]))
				print_r("update book($bookid) error: " . $db->get_error . "\n");
		}

		print_r("db chapters: " . count($dbchapters) . " chapters: " . count($chapters) . "\n");
		foreach($chapters as $chapter){
			$chapterid = $chapter["uri"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				$db->add_chapter(CYSTS8::$siteid, $bookid, $chapterid, $name, "");
			} else {
				$dbchapter = $dbchapters[$chapterid];
				if(strlen($dbchapter["uri"]) > 0)
					continue;
			}
		}
		
		return 0;
	}

	function Action1()
	{
		//1. add book
		$books = GetAll();
		//$books = GetHot();
		//$books = GetUpdate();
		print_r("Get Books: " . count($books) . "\n");
		AddBook($books);
	}

	// add
	function Action2($fast_mode)
	{
		global $db;
		$dbbooks = $db->get_books(CYSTS8::$siteid);
		print_r("db-book count: " . count($dbbooks) . "\n");

		// 2. add chapter
		$i = 0;	
		foreach($dbbooks as $bookid => $dbbook)
		{
			$i++;
			$name = $dbbook["name"];
			print_r("AddChapter([$i]$bookid - $name)\n");
			AddChapter($bookid, $name, $fast_mode);
		}
	}

	// add chapter download task(from database only)
	function Action3()
	{
		$client = new GearmanClient();
		$client->addServer("115.28.54.237", 4730);

		$i = 0;
		global $db;
		$dbbooks = $db->get_books(CYSTS8::$siteid);
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters(CYSTS8::$siteid, $bookid);
			foreach($dbchapters as $chapterid => $dbchapter)
			{
				if(strlen($dbchapter["uri"]) > 1)
					continue;

				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadYsts8', $workload);
			}
		}
	}
?>
