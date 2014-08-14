<?php
	require("php/db.inc");
	require("php/http.inc");

	$db = dbopen("pingshu", "115.28.54.237");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	if(count($argv) < 2)
	{
		print_r("please input server ip");
		return -1;
	}
	Action($argv[1]);

	function Action($ip)
	{
		global $db;
		$sql = sprintf("select bookid, chapterid, uri from chapters");
		$res = $db->query($sql);
		if(!$res)
		{
			print_r("Action failed: " . $db->error);
			return -1;
		}

		$chapters = array();
		while($row = $res->fetch_assoc())
		{
			$bookid = $row["bookid"];
			$chapterid = $row["chapterid"];
			$uri = $row["uri"];

			if(strlen($uri) > 0)
				Download($ip, $bookid, $chapterid, $uri);
		}

		$res->free();
		return $chapters;
	}

	function db_set_chapter_uri($bookid, $chapterid, $uri)
	{
		global $db;
		$sql = sprintf('update chapters set uri="%s" where bookid="%s" and chapterid=%d', $uri, $bookid, $chapterid);
		if(!$db->query($sql))
			print_r("DB set uri failed: " . $db->error);
		return $db->error;
	}

	function Download($ip, $bookid, $chapterid, $uri)
	{
		$server = "";
		$path = "/";
		$urls = explode(":", $uri);
		if(count($urls) == 1){
			$server = "115.28.51.131";
			$path = $uri;
		} else {
			$server = $urls[0];
			$path = $urls[1];
		}

		if(0 != strcmp($ip, $server))
			return;

        $uri = $path;
		$uri = str_replace("//home/pingshu8", "/1", $uri);
		$uri = str_replace("/home/pingshu8", "/1", $uri);
		$uri = str_replace("//ts/pingshu8", "/2", $uri);
		$uri = str_replace("/ts/pingshu8", "/2", $uri);
		$uri = str_replace("//ts2/pingshu8", "/3", $uri);
		$uri = str_replace("/ts2/pingshu8", "/3", $uri);
		$uri = sprintf("http://%s%s", $ip, $uri);
		print_r("Download: $uri\n");
		$audio = HttpDownload($uri);
		if(0 == strlen($audio))
		{
			print_r("Download($uri) failed.\n");
			return -1;
		}

		// write file
		if(!is_dir("/home/pingshu8/$bookid")){
			mkdir("/home/pingshu8/$bookid", 777, true);
		}

		$filename = sprintf("/home/pingshu8/%s/%d.mp3", $bookid, $chapterid);
		file_put_contents($filename, $audio);

		// write db
		db_set_chapter_uri($bookid, $chapterid, "175.195.249.184:$filename");
		return 0;
	}

	function HttpDownload($uri)
	{
		$audio = http_get($uri, 60);
		return $audio;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
