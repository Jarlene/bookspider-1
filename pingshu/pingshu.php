<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("pingshu8.php");
	require("ysts8.php");
	
	$mc = new Memcached();
	$mc->addServer("localhost", 11211);

	$server = php_reqvar("server", '');
	$catalog = php_reqvar("catalog", '');
	$book = php_reqvar("book", '');
	$chapter = php_reqvar("chapter", '');

	$data = array();
	if(0 == strlen($server)){
		$servers = GetServers();
		foreach($servers as $k => $v){
			$data[] = array("id" => $k, "name" => $v["name"]);
		}
	} else {
		$s = GetServerObj($server);
		if(0 == strlen($catalog)){
			$catalogs = GetCatalog($s);
			foreach($catalogs as $k => $v){
				$keys = array();
				foreach($v as $key => $value){
					$keys[] = $key;
				}
				$data[$k] = $keys;
			}
		} else if(0 == strlen($book)){
			$books = GetBooks($s, $catalog);
			foreach($books as $k => $v){
				$data[] = $k;
			}
		} else if(0 == strlen($chapter)){
			$chapters = GetChapters($s, $catalog, $book);
			foreach($chapters as $k => $v){
				$data[] = $k;
			}
		} else {
			$data = GetAudio($s, $catalog, $book, $chapter);
		}
	}

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);

	function GetServers()
	{
		$pingshu8 = new CPingShu8();
		$ysts8 = new CYSTS8();

		$servers = array();
		$servers["0"] = array("name" => "服1", "object" => $pingshu8);
//		$servers["1"] = array("name" => "服2", "object" => $pingshu8);
//		$servers["2"] = array("name" => "服3", "object" => $pingshu8);
//		$servers["3"] = array("name" => "服4", "object" => $pingshu8);
		$servers["4"] = array("name" => "服5", "object" => $ysts8);
		return $servers;
	}

	function GetServerObj($s)
	{
		$servers = GetServers();
		foreach($servers as $k => $v){
			if(0 == strcmp($k, $s))
				return $v["object"];
		}
		return "";
	}

	function GetCatalog($s)
	{
		global $mc;
		$mckey = "ts-server-" . $s->GetName();
		$catalog = $mc->get($mckey);

		if(!$catalog){
			$catalog = $s->GetCatalog();
			$mc->set($mckey, json_encode($catalog), 24*60*60-1);
		} else {
			$catalog = json_decode($catalog, True);
		}

		return $catalog;
	}
	
	function GetCatalogUri($s, $catalog)
	{
		$catalogs = GetCatalog($s);
		foreach($catalogs as $k => $v){
			foreach($v as $key => $value){
				if(0 == strcmp($key, $catalog))
					return $value;
			}
		}
		return "";
	}
	
	function GetBooks($s, $catalog)
	{
		global $mc;
		$mckey = "ts-server-" . $s->GetName() . "-catalog-" . $catalog;
		$books = $mc->get($mckey);

		if(!$books){
			$uri = GetCatalogUri($s, $catalog);
			if(0 == strlen($uri))
				return "";
			$books = $s->GetBooks($uri);
			$mc->set($mckey, json_encode($books), 24*60*60-1);
		} else {
			$books = json_decode($books, True);
		}

		return $books;
	}

	function GetBookUri($s, $catalog, $book)
	{
		$books = GetBooks($s, $catalog);
		foreach($books as $k => $v){
			if(0 == strcmp($k, $book))
				return $v;
		}
		return "";
	}

	function GetChapters($s, $catalog, $book)
	{
		global $mc;
		$mckey = "ts-server-" . $s->GetName() . "-catalog-" . $catalog . "-book-" . $book;
		$chapters = $mc->get($mckey);

		if(!$chapters){
			$uri = GetBookUri($s, $catalog, $book);
			if(0 == strlen($uri))
				return "";
			$chapters = $s->GetChapters($uri);
			$mc->set($mckey, json_encode($chapters), 24*60*60-1);
		} else {
			$chapters = json_decode($chapters, True);
		}

		return $chapters;
	}
	
	function GetChapterUri($s, $catalog, $book, $chapter)
	{
		$chapters = GetChapters($s, $catalog, $book);
		foreach($chapters as $k => $v){
			if(0 == strcmp($k, $chapter))
				return $v;
		}
		return "";
	}

	function GetAudio($s, $catalog, $book, $chapter)
	{
		global $mc;
		$mckey = "ts-server-" . $s->GetName() . "-catalog-" . $catalog . "-book-" . $book . "-chapter-" . $chapter;
		$audio = $mc->get($mckey);

		if(!$audio){
			$uri = GetChapterUri($s, $catalog, $book, $chapter);
			if(0 == strlen($uri))
				return "";
			$audio = $s->GetAudio($uri);
			$mc->set($mckey, $audio, 5*60);
		} else {
			//$chapters = json_decode($chapters, True);
		}

		return $audio;
	}

	// print_r($all);
	//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm'));
	//print_r(ysts8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
	//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
