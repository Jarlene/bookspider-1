<?php
	$server = php_reqvar("server", '');
	$catalog = php_reqvar("catalog", '');
	$bookid = php_reqvar("bookid", '');
	$chapter = php_reqvar("chapterid", '');
	$keyword = php_reqvar("keyword", '');
	$redirect = php_reqvar("redirect", 0);

	// HTTP Headers
	$headers = array();
	$headers["User-Agent"] = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/28.0";

	// Reply
	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["headers"] = $headers;

	// memory DB
	$mdb = new Redis();
	$mdb->connect('127.0.0.1', 6379);

	$data = array();
	if(0 != $redirect){
		$reply["bookid"] = $bookid;
		$reply["chapterid"] = $chapter;
		$data = OnRedirect($server, $redirect);
	} else if(strlen($keyword) > 0){
		$data = OnSearch($server, $keyword);
	} else if(0 == strlen($server)){
		$servers = GetServers();
		foreach($servers as $k => $v){
			$data[] = array("id" => "$k", "name" => $v);
		}
	} else {
		$s = GetServerObj($server);
		if(strlen($chapter) > 0){
			$reply["catalog"] = $catalog;
			$reply["bookid"] = $bookid;
			$reply["chapterid"] = $chapter;
			if(1 == $s->redirect)
				$reply["code"] = 300;
			$data = GetPictures($s, $catalog, $bookid, $chapter);
		} else if(strlen($bookid) > 0) {
			$chapters = GetChapters($s, $catalog, $bookid);
			$reply["icon"] = $chapters["icon"];
			$reply["summary"] = $chapters["summary"];
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
				$data[] = $k;
			}
		}
	}

	$reply["data"] = $data;
	echo json_encode($reply);

	////////////////////////////////////////////////////////////////////////////////////
	// Redirect
	function OnRedirect($server, $redirect)
	{
		$s = GetServerObj($server);
		$req = file_get_contents("php://input");
		return $s->GetAudio($bookid, $chapter, $req);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Search
	function OnSearch($server, $keyword)
	{
		$data = array();
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
		return $data;
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

	////////////////////////////////////////////////////////////////////////////////////
	// Servers
	function GetServers()
	{
		return array("0" => "服务器1",  // imanhua
					"1" => "服务器2", // 99comic
					"2" => "服务器3"); // veryim
		return $servers;
	}

	function GetServerObj($s)
	{
		$id = (int)$s;
		if(0 == $id){
			include('imanhua.php');
			return new CIManHua();
		} else if(1 == $id){
			include('veryim.php');
			return new CVeryIm;
		} else {
			return "";
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Catalog
	function GetCatalog($s)
	{
		global $mdb;
		$mdbkey = "comic-" . $s->GetName();
		$catalog = $mdb->get($mdbkey);

		if(!$catalog){
			$catalog = $s->GetCatalog();
			if(count($catalog) > 2)
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
			if(0 == strcmp($k, $catalog))
				return $v;
		}
		return "";
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Books
	function GetBooks($s, $catalog)
	{
		global $mdb;
		$mdbkey = "comic-" . $s->GetName() . "-" . $catalog;
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

	////////////////////////////////////////////////////////////////////////////////////
	// Chapters
	function GetChapters($s, $catalog, $bookid)
	{
		global $mdb;
		$mdbkey = "comic-" . $s->GetName() . "-" . $catalog . "-" . $bookid;
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

	////////////////////////////////////////////////////////////////////////////////////
	// Pictures
	function GetPictures($s, $catalog, $bookid, $chapter)
	{
		global $mdb;
		if(0 != $s->cache["audio"]){
			$mdbkey = "comic-" . $s->GetName() . "-" . $catalog . "-" . $bookid . "-" . $chapter;
			$pictures = $mdb->get($mdbkey);
		} else {
			$pictures = null;
		}

		if(!$pictures){
			$uri = GetChapterUri($s, $catalog, $bookid, $chapter);
			if(0 == strlen($uri))
				return "";

			if(1 == $s->redirect)
				return $uri; // client get remote content

			$pictures = $s->GetPictures($bookid, $chapter, $uri);

			if(count($pictures) > 0 && 0 != $s->cache["audio"])
				$mdb->set($mdbkey, json_encode($pictures), $s->cache["audio"]);
		} else {
			$pictures = json_decode($pictures, True);
		}

		return $audio;
	}
	
	function DBSave()
	{
		$mdb = new Redis();
		$mdb->connect('127.0.0.1', 6379);

		$name = $this->GetName();
		$catalog = $this->GetCatalog();
		foreach($catalog as $k => $v){
			print_r("[$name] catalog: $v\n");
			for($i = 0; $i < 10; $i++){
				$books = $this->GetBooks($v);
				print_r("[$name] books: ". count($books["book"]). "\n");
				if(count($books["book"]) < 1){
					sleep(5);
					continue;
				}

				$mdb->set("comic-$name-$k", json_encode($books));					
				break;
			}
		}
	}
?>
