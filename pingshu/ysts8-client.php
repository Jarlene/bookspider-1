<?php
	require_once("php/dom.inc");
	require_once("php/http.inc");
	require_once("http-proxy.php");
	require_once("db-pingshu.inc");
	require_once("ysts8.php");

	$db = new DBPingShu("115.28.54.237");

	$update_mode = False;
	Action1($update_mode);
	Action2($update_mode);
	Action3();

	function Action1($update_mode)
	{
		//1. add book
		$books = GetBooks($update_mode);
		//$books = GetHot();
		//$books = GetUpdate();
		print_r("Get Books: " . count($books) . "\n");

 		$i = 0;
		global $db;
		$dbbooks = $db->get_books(CYSTS8::$siteid);
		foreach($books as $id => $name)
		{
			$i++;
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("[$i]DB add book($id, $name)\n");
				if(0 != $db->add_book(CYSTS8::$siteid, $id, "", $name, "", "", "", ""))
					print_r("add book($id) error: " . $db->get_error . "\n");
			} 
		}
	}

	// add
	function Action2($update_mode)
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
			AddChapter($bookid, $name, $update_mode);
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
				$uri = $dbchapter["uri"];
				$uri2 = $dbchapter["uri2"];
				if(strlen($uri) > 1 || strlen($uri2) > 1)
					continue;

				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadYsts8', $workload, $workload);
			}
		}
	}

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function GetBooks($update_mode)
	{
		$books = array();
		if($update_mode){
			$books = _WebGetBooks("http://www.ysts8.com/index_tim.html");
		} else {
			$site = new CYSTS8();
			$catalogs = $site->GetCatalog();
			$catalogs = $catalogs["小说"];
			foreach ($catalogs as $name => $value) {
				$catalog = $site->GetBooks($value);
				$books = array_merge($books, $catalog["book"]);
			}
		}
		return $books;
	}

	function AddChapter($bookid, $bookname, $update_mode)
	{
		global $db;
		global $client;

		$dbchapters = $db->get_chapters(CYSTS8::$siteid, $bookid);
		if(!$update_mode && count($dbchapters) > 0){
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
	
	//----------------------------------------------------------------------------
	// Website
	//----------------------------------------------------------------------------	
	function _WebGetBooks($uri)
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

?>
