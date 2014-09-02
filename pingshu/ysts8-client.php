<?php
	require_once("php/dom.inc");
	require_once("php/http.inc");
	require_once("http-proxy.php");
	require_once("db-pingshu.inc");
	require_once("ysts8.php");

	date_default_timezone_set("Asia/Shanghai");

	$db = new DBPingShu("115.28.54.237");
	$site = new CYSTS8();
	$client = new GearmanClient();
	$client->addServer("115.28.54.237", 4730);

	$update_mode = False;
	Action1($update_mode);
	Action2($update_mode);
	Action3();

	function Action1($update_mode)
	{
		//1. add book
		$i = 0;
		$urls = GetBooks($update_mode);
		print_r("Get urls: " . count($urls) . "\n");
		//$books = GetHot();
		//$books = GetUpdate();

		global $db;
		global $site;
		$dbbooks = $db->get_books(CYSTS8::$siteid);
		foreach($urls as $uri){
			$books = $site->WebGetBooks($uri);
			print_r("[$uri] books: " . count($books) . "\n");
			foreach($books as $id => $name)
			{
				$i++;
				if(!array_key_exists($id, $dbbooks)){
					print_r("[$i]DB add book($id, $name)\n");
					if(0 != $db->add_book(CYSTS8::$siteid, $id, "", $name, "", "", "", "")){
						print_r("add book($id) error: " . $db->get_error() . "\n");
						die();
					}
				} else {
					$dbbook = $dbbooks[$id];
					if(strlen($dbbook["name"]) < 1){
						print_r("[$i]DB book update($id, $name)\n");
						if(0 != $db->update_book(CYSTS8::$siteid, $id, $dbbook["author"], $name, $dbbook["icon"], $dbbook["summary"], $dbbook["catalog"], $dbbook["subcatalog"])){
							print_r("update book($id) error: " . $db->get_error() . "\n");
							die();
						}
					}
				}
			}
			sleep(10);
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
			//if($i < 10000) continue;

			$name = $dbbook["name"];
			print_r("AddChapter([$i]$bookid - $name)\n");
			if(0 != AddChapter($bookid, $name, $update_mode))
				sleep(20);
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

				++$i;
				print_r("[$i]Add task($bookid, $chapterid)\n");
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
		global $site;
		$urls = array();
		if($update_mode){
			$urls[] = "http://www.ysts8.com/index_tim.html";
		} else {
			$catalogs = $site->GetCatalog();
			$catalogs = $catalogs["小说"];
			foreach ($catalogs as $name => $value) {
				$pages = $site->GetCatalogUrls($value);
				$urls = array_merge($urls, $pages);
				sleep(5);
			}
		}

		return $urls;
	}

	function AddChapter($bookid, $bookname, $update_mode)
	{
		global $db;
		global $site;
		global $client;

		$dbchapters = $db->get_chapters(CYSTS8::$siteid, $bookid);
		if(!$update_mode && count($dbchapters) > 0){
			print_r("book has download.\n");
			return 0;
		}

		$result = $site->GetChapters($bookid);
		$chapters = $result["chapter"];
		if(count($chapters) < 1){
			print_r("GetChapters($bookid) error.\n");
			return -1;
		} else if(count($dbchapters) == count($chapters)){
			print_r("book has download.\n");
			return 1;
		}

		if(count($dbchapters)<1 && strlen($result["info"]) > 1){
			print_r("DB book update($bookid, $bookname)\n");
			if(0 != $db->update_book(CYSTS8::$siteid, $bookid, "", $bookname, $result["icon"], $result["info"], $result["catalog"], $result["subcatalog"])){
				print_r("update book($bookid) error: " . $db->get_error() . "\n");
				die();
			}
		}

		print_r("db chapters: " . count($dbchapters) . " chapters: " . count($chapters) . "\n");
		foreach($chapters as $chapter){
			$chapterid = $chapter["uri"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				if(0 != $db->add_chapter(CYSTS8::$siteid, $bookid, $chapterid, $name, "")){
					print_r("add_chapter($bookid, $chapterid) error: " . $db->get_error() . "\n");
					die();
				}
	
				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('DownloadYsts8', $workload, $workload);
			} else {
				$dbchapter = $dbchapters[$chapterid];
				if(strlen($dbchapter["uri"]) > 0 || strlen($dbchapter["uri2"]) > 0)
					continue;
			}
		}

		return 1;
	}
?>
