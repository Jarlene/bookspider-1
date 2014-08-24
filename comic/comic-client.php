<?php
require_once("bengou.php");
//require_once("imanhua.php");
require_once("db-comic.inc");

$db = new DBComic("115.28.51.131");

$sites = array(
	CBenGou::$siteid => new CBenGou(),
//	CIManHua::$siteid => new CIManHua(),
);

Action1();
Action2(False);
Action3();

function Action1()
{
	global $sites;
	foreach($sites as $siteid => $site){
		$books = $site->ListBook();
		DBAddBooks($siteid, $books);
	}
}

function Action2($update_mode)
{
	global $db;
	global $sites;
	foreach($sites as $siteid => $site){
		$dbbooks = $db->get_books($siteid);
		print_r("[$siteid] db-book count: " . count($dbbooks) . "\n");

		$i = 0;	
		foreach($dbbooks as $bookid => $dbbook)
		{
			$i++;
			$name = $dbbook["name"];
			print_r("AddChapter([$siteid:$i]$bookid - $name)\n");
			DBAddChapters($siteid, $site, $bookid, $dbbook, $update_mode);
		}
	}
}

// add chapter download task(from database only)
function Action3()
{
	$client = new GearmanClient();
	$client->addServer("115.28.54.237", 4730);

	$i = 0;
	global $db;
	global $sites;
	foreach($sites as $siteid => $site){
		$dbbooks = $db->get_books($siteid);
		print_r("[$siteid] db-book count: " . count($dbbooks) . "\n");
		foreach($dbbooks as $bookid => $dbbook)
		{
			$dbchapters = $db->get_chapters($siteid, $bookid);
			print_r("[$siteid] db-chapter count: " . count($dbchapters) . "\n");
			foreach($dbchapters as $chapterid => $dbchapter)
			{
				print_r("Add task($bookid, $chapterid)\n");
				$workload = sprintf("%s,%d", $bookid, $chapterid);
				$client->doBackground('comic-bengou', $workload, $workload);
			}
		}
	}
}

//----------------------------------------------------------------------------
// DB Functions
//----------------------------------------------------------------------------		
function DBAddBooks($siteid, $books)
{
	global $db;
	$i = 0;
	$dbbooks = $db->get_books($siteid);
	foreach($books as $bookid => $book)
	{
		$i++;
		if(!array_key_exists($bookid, $dbbooks))
		{
			$name = $book["name"];
			print_r("[$i]DB add book($bookid, $name)\n");
			if(0 != $db->add_book($siteid, $bookid, $book["author"], $book["name"], $book["icon"], $book["summary"], "", "", "", $book["date"]))
				print_r("add book($bookid) error: " . $db->get_error() . "\n");
		} 
	}
}

function DBAddChapters($siteid, $site, $bookid, $dbbook, $update_mode)
{
	global $db;
	global $client;

	$dbchapters = $db->get_chapters($siteid, $bookid);
	if(!$update_mode && count($dbchapters) > 0){
		print_r("DBAddChapters: book has download.\n");
		return 0;
	}

	$book = $site->GetBook($bookid);
	if(False === $book){
		print_r("DBAddChapters($bookid) error.\n");
		return 0;
	}

	if(strlen($dbbook["summary"]) < 1 && strlen($book["summary"]) > 1){
		print_r("DB book update($bookid, $bookname)\n");
		if(0 != $db->update_book($siteid, $bookid, $book["author"], $book["name"], $book["icon"], $book["summary"], $book["region"], $book["catalog"], $book["tags"], $book["date"]))
			print_r("update book($bookid) error: " . $db->get_error() . "\n");
	}

	$sections = $book["section"];
	foreach($sections as $secion){
		$chapters  = $secion["chapters"];
		foreach($chapters as $chapter){
			$chapterid = $chapter["id"];
			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)\n");
				$db->add_chapter($siteid, $bookid, $chapterid, $name, $secion["name"]);
			}
		}
	}
}
?>
