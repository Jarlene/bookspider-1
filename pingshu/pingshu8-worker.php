<?php
	require("php/db.inc");
	require("php/sys.inc");
	require("php/http.inc");

	$db = dbopen("pingshu", "115.28.54.237");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	$task_count = 0;
	$ip = "";
	$servers = array("115.28.51.131", "115.28.54.237", "115.29.145.111", "112.126.69.201", "121.40.136.6", "175.195.249.184");
	$ips = get_network_interface();
	foreach($ips as $net){
		if(in_array($net["ip"], $servers)){
			$ip = $net["ip"];
			break;
		}
	}
	if(strlen($ip) < 1)
	{
		print_r("server ip error.");
		return -1;
	}

	$paths = array("115.28.51.131" => "/ts2/pingshu8/", "175.195.249.184" => "/home/pingshu8/");
	if(!array_key_exists($ip, $paths)){
		$basedir = "/ts/pingshu8/";
	} else {
		$basedir = $paths[$ip];
	}
	
	print_r("IP: $ip\n");
	print_r("DIR: $basedir\n");

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
		$sql = sprintf('update pingshu8 set uri="%s" where bookid="%s" and chapterid=%d', $uri, $bookid, $chapterid);
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
		global $ip;
		global $basedir;
		global $task_count;
		$task_count++;
		print_r("Action[$task_count]: $bookid, $chapterid\n");
		$audio = Download($bookid, $chapterid);
		if(0 == strlen($audio))
		{
			print_r("Download($uri) failed.\n");
			return -1;
		}

		// write file
		$dir = "$basedir$bookid";
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
		db_set_chapter_uri($bookid, $chapterid, "$ip:$filename");
		return 0;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
