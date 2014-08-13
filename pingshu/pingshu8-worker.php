<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/api.inc");
	require("php/dom.inc");
	require("php/http.inc");
	require("http-proxy.php");
	require("php/http-multiple.inc");
	require("http-multiple-proxy.php");
	//require("pingshu8.php");

	$db = dbopen("pingshu", "115.28.54.237");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	$worker = new GearmanWorker();
	$worker->addServer('115.28.54.237', 4730);
	$worker->addFunction('DownloadPingshu8', 'DoDownload');
	while ($worker->work());

	function DoDownload($job) {
		$args = $job->workload();
		list($bookid, $chapterid) = explode(",", $args);

		//$bookid = "7_3283_1";
		//$chapterid = 161405;
		return Action($bookid, $chapterid);
	}

	function db_set_chapter_uri($bookid, $chapterid, $uri)
	{
		global $db;
		$sql = sprintf('update chapters set uri="%s" where bookid="%s" and chapterid=%d', $uri, $bookid, $chapterid);
		if(!$db->query($sql))
			print_r("DB set uri failed: " . $db->error);
		return $db->error;
	}

	function Download($bookid, $chapterid)
	{
		//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
		$uri = "http://www.pingshu8.com/bzmtv_Inc/download.asp?fid=$chapterid";
		//$referer = "referer: http://www.pingshu8.com/Play_Flash/js/Jplayer.swf";
		$referer = "referer: http://www.pingshu8.com/down_$chapterid.html";
		$audio = http_get($uri, 120, "", array($referer));
		return $audio;
	}

	function Action($bookid, $chapterid)
	{
		print_r("Action: $bookid, $chapterid\n");
		$audio = Download($bookid, $chapterid);
		if(0 == strlen($audio))
		{
			print_r("Download($uri) failed.\n");
			return -1;
		}

		// write file
		$dir = "/ts/pingshu8/$bookid";
		if(!is_dir($dir)){
			mkdir($dir, 0777, true);
		}

		$mb = 200 * 1024 * 1024; // 200MB
		if(disk_free_space($dir) < $mb){
			print_r("disk full($mb)\n");
			die();
		}

		$filename = sprintf("$dir/%d.mp3", $chapterid);
		file_put_contents($filename, $audio);

		// write db
		db_set_chapter_uri($bookid, $chapterid, "115.29.145.111:$filename");
		return 0;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
