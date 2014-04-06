<?php
	require("php/dom.inc");
	require("php/util.inc");
	require("php/http.inc");
	require("pingshu8.php");
	require("ysts8.php");
	require("77nt.php");
	require("17tsw.php");

	$mdb = new Redis();
	$mdb->connect('127.0.0.1', 6379);

	$server = php_reqvar("server", '');
	$catalog = php_reqvar("catalog", '');
	$bookid = php_reqvar("bookid", '');
	$chapter = php_reqvar("chapterid", '');
	$keyword = php_reqvar("keyword", '');
	$redirect = php_reqvar("redirect", 0);

	$reply["code"] = 0;
	$reply["msg"] = "ok";

	$data = array();
	if(0 != $redirect){
		$s = GetServerObj($server);
		$req = file_get_contents("php://input");
		$data = $s->GetAudio($bookid, $chapter, $req);
		$reply["bookid"] = $bookid;
		$reply["chapterid"] = $chapter;
	} else if(strlen($keyword) > 0){
		$servers = GetServers();
		foreach($servers as $k => $v){
			if( (strlen($server) > 0 && 0 != strcmp($k, $server)) || 2==$k || 3==$k)
				continue;
			$result = Search($v["object"], $keyword);
			foreach($result as $b){
				$bid = $b["bookid"];
				$data[] = array("server" => $k, "book" => $b["book"], "bookid" => "$bid");
			}
		}
	} else if(0 == strlen($server)){
		$servers = GetServers();
		foreach($servers as $k => $v){
			$data[] = array("id" => "$k", "name" => $v["name"]);
		}
	} else {
		$s = GetServerObj($server);
		if(strlen($chapter) > 0){
			$reply["catalog"] = $catalog;
			$reply["bookid"] = $bookid;
			$reply["chapterid"] = $chapter;
			if(1 == $s->redirect)
				$reply["code"] = 300;
			$data = GetAudio($s, $catalog, $bookid, $chapter);
		} else if(strlen($bookid) > 0) {
			$chapters = GetChapters($s, $catalog, $bookid);
			$reply["icon"] = $chapters["icon"];
			$reply["summary"] = $chapters["info"];
			$i = 1;
			foreach($chapters["chapter"] as $v){
				$data[] = array("chapter" => $v["name"], "chapterid" => "$i");
				$i++;
			}
		} else if(strlen($catalog) > 0) {
			$books = GetBooks($s, $catalog);
			$reply["icon"] = $books["icon"];
			foreach($books["book"] as $k => $v){
				$data[] = array("book" => $v, "bookid" => "$k");
			}
		} else {
			$catalogs = GetCatalog($s);
			foreach($catalogs as $k => $v){
				$keys = array();
				foreach($v as $key => $value){
					$keys[] = $key;
				}
				$data[$k] = $keys;
			}
		}
	}

	$reply["data"] = $data;
	echo json_encode($reply);

	function GetServers()
	{
		$pingshu8 = new CPingShu8();
		$ysts8 = new CYSTS8();
		$c77nt = new C77NT();
		$c17tsw = new C17TSW();

		$servers = array();
		$servers["4"] = array("name" => "服务器5", "object" => $ysts8);
		$servers["0"] = array("name" => "服务器1", "object" => $pingshu8);
//		$servers["1"] = array("name" => "服务器2", "object" => $c77nt);
		$servers["2"] = array("name" => "服务器3", "object" => $c77nt);
		$servers["3"] = array("name" => "服务器4", "object" => $c17tsw);
		return $servers;
	}

	function GetServerObj($s)
	{
		$servers = GetServers();
		foreach($servers as $k => $v){
			if(0 == strcmp($k, $s))
				return $v["object"];
		}
		return false;
	}

	function GetCatalog($s)
	{
		global $mdb;
		$mdbkey = "ts-server-" . $s->GetName();
		$catalog = $mdb->get($mdbkey);

		if(!$catalog){
			$catalog = $s->GetCatalog();
			$ok = true;
			foreach($catalog as $k => $v){
				if(count($v) < 1)
					$ok = false;
			}
			if($ok)
				$mdb->set($mdbkey, json_encode($catalog), $s->cache["catalog"]);
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
		global $mdb;
		$mdbkey = "ts-server-" . $s->GetName() . "-catalog-" . $catalog;
		$books = $mdb->get($mdbkey);

		if(!$books){
			$uri = GetCatalogUri($s, $catalog);
			if(0 == strlen($uri))
				return "";
			$books = $s->GetBooks($uri);
			if(count($books["book"]) > 0)
				$mdb->set($mdbkey, json_encode($books), $s->cache["book"]);
		} else {
			$books = json_decode($books, True);
		}

		return $books;
	}

	// function GetBookUri($s, $catalog, $bookid)
	// {
		// $books = GetBooks($s, $catalog);
		// foreach($books["book"] as $k => $v){
			// if(0 == strcmp($k, $bookid))
				// return $v;
		// }
		// return "";
	// }

	function GetChapters($s, $catalog, $bookid)
	{
		global $mdb;
		$mdbkey = "ts-server-" . $s->GetName() . "-catalog-" . $catalog . "-book-" . $bookid;
		$chapters = $mdb->get($mdbkey);

		if(!$chapters){
			// $uri = GetBookUri($s, $catalog, $book);
			// if(0 == strlen($uri))
				// return "";
			$chapters = $s->GetChapters($bookid);
			if(count($chapters["chapter"]) > 0)
				$mdb->set($mdbkey, json_encode($chapters), $s->cache["chapter"]);
		} else {
			$chapters = json_decode($chapters, True);
		}

		return $chapters;
	}
	
	function GetChapterUri($s, $catalog, $bookid, $chapter)
	{
		$chapters = GetChapters($s, $catalog, $bookid);
		// foreach($chapters["chapter"] as $k => $v){
			// if(0 == strcmp($k, $chapter))
				// return $v;
		// }
		//return "";
		return $chapters["chapter"][$chapter-1]["uri"];
	}

	function GetAudio($s, $catalog, $bookid, $chapter)
	{
		global $mdb;
		if(0 != $s->cache["audio"]){
			$mdbkey = "ts-server-" . $s->GetName() . "-catalog-" . $catalog . "-book-" . $bookid . "-chapter-" . $chapter;
			$audio = $mdb->get($mdbkey);
		}

		if(!$audio){
			$uri = GetChapterUri($s, $catalog, $bookid, $chapter);
			if(0 == strlen($uri))
				return "";
				
			if(1 == $s->redirect)
				return $uri; // client get remote content

			$audio = $s->GetAudio($bookid, $chapter, $uri);

			if(0 != $s->cache["audio"] && strlen($audio) > 0)
				$mdb->set($mdbkey, $audio, $s->cache["audio"]);
		} else {
			//$chapters = json_decode($chapters, True);
		}

		return $audio;
	}
	
	function Search($s, $keyword)
	{
		$data = array();
		$result = $s->Search($keyword);
		if(count($result) > 1){
			foreach($result["catalog"] as $catalog){
				$books = GetBooks($s, $catalog);
				foreach($books["book"] as $k => $v){
					$data[] = array("book" => $v, "bookid" => $k);
				}
			}
		}
		foreach($result["book"] as $k => $v){
			$data[] = array("book" => $v, "bookid" => $k);
		}
		return $data;
	}

	// print_r($all);
	//print_r(pingshu8_artist('http://www.pingshu8.com/Music/bzmtv_1.Htm'));
	//print_r(ysts8_artist('http://www.pingshu8.com/Music/bzmtv_2.Htm'));
	//print_r(ysts8_works('http://www.ysts8.com/Ysmp3/40_1.html'));
	//print_r(ysts8_chapters('http://www.ysts8.com/Yshtml/Ys12073.html'));
	//print_r(ysts8_audio('http://www.ysts8.com/play_12073_46_2_1.html'));
?>
