<?php
	require("php/dom.inc");
	require("php/util.inc");
	require("php/http.inc");
	require("php/http-multiple.inc");

	require("http-proxy.php");

	require("http-multiple-proxy.php");
	require("pingshu8.php");
	require("ysts8.php");
	require("aitingwang.php");
	require("77nt.php");
	require("17tsw.php");	
	require("tingvv.php");	
	
	$mdb = new Redis();
	$mdb->connect('127.0.0.1', 6379);

	$server = php_reqvar("server", '');
	$catalog = php_reqvar("catalog", '');
	$bookid = php_reqvar("bookid", '');
	$chapter = php_reqvar("chapterid", '');
	$keyword = php_reqvar("keyword", '');
	$g_redirect = php_reqvar("redirect", 0);

	$reply["code"] = 0;
	$reply["msg"] = "ok";

	$headers = array();
	$headers["User-Agent"] = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/28.0";

	$data = array();
	if(0 != $g_redirect)
	{
		$s = GetServerObj($server);
		$req = file_get_contents("php://input");
		if(0 == strcmp("5", $server))
		{
			$data1 = $s->GetAudio($bookid, $chapter, $req); // user-defined HTTP Header
			$data = $data1["url"];
			$headers = $data1["headers"];
		} 
		else if(0 == strcmp("0", $server))
		{
			//$headers["Referer"] = "http://www.pingshu8.com/Play_Flash/js/Jplayer.swf";
			$headers["User-Agent"] = "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)";
 			$headers["User-Agent"] = getRandomUserAgent();
			$data = $s->GetAudio($bookid, $chapter, $req);
		}
		else 
		{
			$data = $s->GetAudio($bookid, $chapter, $req);
		}
		
		$reply["bookid"] = $bookid;
		$reply["chapterid"] = $chapter;
	}
	else if(strlen($keyword) > 0)
	{
		$servers = GetServers();
		foreach($servers as $k => $v){
			if(0 != strcmp("0", $k) )
				continue;
			$result = Search($v["object"], $keyword);
			foreach($result as $b){
				if(strpos($b["book"], "ÂìàÂà©") === false){
					$bid = $b["bookid"];
					$data[] = array("server" => $k, "book" => $b["book"], "bookid" => "$bid");
				}
			}
		}
	} 
	else if(0 == strlen($server)){
		$servers = GetServers();
		foreach($servers as $k => $v){
			$data[] = array("id" => "$k", "name" => $v["name"]);
		}
	} else {
		$s = GetServerObj($server);
		if(strlen($chapter) > 0)
		{
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
				if(strpos($v, "ÂìàÂà©") === false){
					$data[] = array("book" => $v, "bookid" => "$k");
				}
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

	$reply["headers"] = $headers;	
	$reply["data"] = $data;
	echo json_encode($reply);

	function GetServers()
	{
		$pingshu8 = new CPingShu8();
		$ysts8 = new CYSTS8();
		$aitingwang = new AITINGWANG();
		$c77nt = new C77NT();
		$c17tsw = new C17TSW();
		$tingvv = new TINGVV();

		$servers = array();
		$servers["0"] = array("name" => "服务器1", "object" => $pingshu8);
// 		$servers["4"] = array("name" => "服务器5", "object" => $ysts8);
 		$servers["5"] = array("name" => "服务器6", "object" => $aitingwang);
// 		$servers["2"] = array("name" => "服务器3", "object" => $c77nt);
// 		$servers["3"] = array("name" => "服务器4", "object" => $c17tsw);
// 		$servers["6"] = array("name" => "服务器7", "object" => $tingvv);
		
		
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
		//return "";
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
