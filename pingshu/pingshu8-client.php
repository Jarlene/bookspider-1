<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/api.inc");
	require("php/dom.inc");
	require("php/http.inc");
	require("http-proxy.php");
	require("php/http-multiple.inc");
	require("http-multiple-proxy.php");
	require("pingshu8.php");

	$db = dbopen("pingshu");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	$client = new GearmanClient();
	$client->addServer();

	Action();

	//----------------------------------------------------------------------------
	// functions
	//----------------------------------------------------------------------------
	function db_add_book($bookid, $name)
	{
		global $db;
		$sql = sprintf('insert into books (bookid, author, name, updatetime) values ("%s", "", "%s", "")', $bookid, $name);
		$res = $db->query($sql);
		return 0;
	}

	function db_add_chapter($bookid, $chapterid, $name, $uri)
	{
		global $db;
		$sql = sprintf("insert into chapters (bookid, chapterid, name, uri) values (\"%s\", %d, \"%s\", \"%s\")", $bookid, $chapterid, $name, $uri);
		$res = $db->query($sql);
		return 0;
	}

	function db_getbooks()
	{
		$books = array();

		global $db;
		$sql = "select bookid from books";
		$res = $db->query($sql);

		$comics = array();
		while($row = $res->fetch_assoc())
		{
			$book = array();
			$bookid = $row["bookid"];
			$books[$bookid] = $bookid; // add book
		}

		$res->free();
		return $books;
	}

	function db_getchapters($bookid)
	{
		$chapters = array();

		global $db;
		$sql = sprintf("select chapterid, uri from chapters where bookid=%d", $bookid);
		$res = $db->query($sql);

		$chapters = array();
		while($row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapterid = $row["chapterid"];
			$uri = $row["uri"];
			$chapters[$chapterid] = $uri;
		}

		$res->free();
		return $chapters;
	}

	function GetTopPingshu()
	{
		$books = array();

		$uri = "http://www.pingshu8.com/top/pingshu.htm";
		$html = http_proxy_get($uri, "luckyzz@163.com", 10);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return $books;

		$xpath = new XPath($html);
		$elements = $xpath->query("//div[@class='tab3']/a");
		foreach ($elements as $element) {
			$href = $element->getattribute('href');
			$book = $element->nodeValue;

			$bookid = basename($href);
			$n = strpos($bookid, '.');
			$books[substr($bookid, 4, $n-4)] = $book;
		}

		return $books;
	}

	function DownloadBook($bookid)
	{
		global $client;

		$obj = new CPingShu8();
		$chapters = $obj->GetChapters($bookid);
		$chapters = $chapters["chapter"];

		$dbchapters = db_getchapters($bookid);
		print_r("db chapters: " . count($dbchapters) . "chapters: " . count($chapters));
		foreach($chapters as $chapter){
			$uri = $chapter["uri"];
			$chapterid = basename($uri, ".html");
			$n = strpos($chapterid, '_');
			$chapterid = substr($chapterid, $n+1);

			if(!array_key_exists($chapterid, $dbchapters)){
				$name = $chapter["name"];
				print_r("DB add chapter($bookid, $chapterid, $name)");
				db_add_chapter($bookid, $chapterid, $name, "");
			} else {
				$file = $dbchapters[$chapterid];
				if(strlen($file) > 0)
					continue;
			}

			print_r("Add task($bookid, $chapterid)");
			$workload = sprintf("%s,%d", $bookid, $chapterid);
			$client->doBackground('DownloadPingshu8', $workload);
		}
	}

	function Action()
	{
		$obj = new CPingShu8();
		//$books = $obj->GetTopPingshu();
		$books = GetTopPingshu();

		$dbbooks = db_getbooks();
		foreach($books as $id => $name)
		{
			if(!array_key_exists($id, $dbbooks))
			{
				print_r("DB add book($id, $name)");
				db_add_book($id, $name);
			}

			print_r("Download: $id");
			DownloadBook($id);
			break;
		}
	}
?>
