<?php
	require("php/db.inc");
	require("php/sys.inc");
	require("php/http.inc");

	$http = new Http();
	$http->settimeout(120);

	$task_count = 0;
	$basedir = "/ts/pingshu8/";
	$paths = array("115.28.51.131" => "/ts2/pingshu8/", "175.195.249.184" => "/home/pingshu8/");
	$ips = get_network_interface();
	foreach($ips as $net){
		if(array_key_exists($net["ip"], $paths)){
			$basedir = $paths[$net["ip"]];
			break;
		}
	}
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

	function Download($bookid, $chapterid)
	{
		global $http;
		//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
		$uri = "http://www.pingshu8.com/bzmtv_Inc/download.asp?fid=$chapterid";
		//$referer = "http://www.pingshu8.com/Play_Flash/js/Jplayer.swf";
		$referer = "http://www.pingshu8.com/down_$chapterid.html";
		$audio = $http->get($uri, array('referer' => $referer));
		return $audio;
	}

	function Action($bookid, $chapterid)
	{
		global $basedir;
		global $task_count;
		$task_count++;
		print_r("Action[$task_count]: $bookid, $chapterid\n");
		$audio = Download($bookid, $chapterid);
		if(0 == strlen($audio))
		{
			print_r("Download($bookid, $chapterid) failed.\n");
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
		return 0;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
