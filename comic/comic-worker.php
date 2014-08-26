<?php
	require_once("php/db.inc");
	require_once("php/sys.inc");
	require_once("php/dom.inc");
	require_once("php/http.inc");
	require("bengou.php");
//	require("imanhua.php");

	$sites = array(
		CBenGou::$siteid => new CBenGou("proxy.cfg2"),
	//	CIManHua::$siteid => new CIManHua(),
	);

	$http = new Http();
//	$http->setcookie("/var/ysts8.cookie");
	$http->settimeout(120);

	$ip = "192.168.164.128";
	$task_count = 0;
	$servers = explode(",", file_get_contents("hosts.cfg"));
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

	$basedir = "/comic/bengou/";
	$paths = array("115.28.51.131" => "/ts2/comic/bengou/");
	$ips = get_network_interface();
	foreach($ips as $net){
		if(array_key_exists($net["ip"], $paths)){
			$basedir = $paths[$net["ip"]];
			break;
		}
	}

	print_r("IP: $ip\n");
	print_r("DIR: $basedir\n");

	$failedCount = 0;
	$client = new GearmanClient();
	$client->addServer("115.28.54.237", 4730);

	$worker = new GearmanWorker();
	$worker->addServer('115.28.54.237', 4730);
	$worker->addFunction('comic-bengou', 'DownloadBengou');
//	$worker->addFunction('comic-imanhua', 'DownloadImanhua');
	while ($worker->work());

	function DownloadBengou($job) {

		$args = $job->workload();
		list($bookid, $chapterid) = explode(",", $args);

		//$bookid = "4182";
		//$chapterid = 0005012703;
		return Action(CBenGou::$siteid, $bookid, intval($chapterid));
	}

	function Action($siteid, $bookid, $chapterid)
	{
		global $basedir;
		global $task_count;
		$task_count++;
		print_r("Action[$task_count]: $bookid, $chapterid\n");
		$images = Download($siteid, $bookid, $chapterid);
		if(False === $images)
		{
			print_r("Download($bookid, $chapterid) failed.\n");
			return -1;
		}

		// write file
		$dir = "$basedir$bookid/$chapterid";
		if(!is_dir($dir)){
			mkdir($dir, 0777, true);
		}

		$mb = 200 * 1024 * 1024; // 200MB
		if(disk_free_space($dir) < $mb){
			print_r("disk full($mb)\n");
			die();
		}

		$i = 0;
		foreach($images as $image){
			$filename = sprintf("$dir/%d.jpg", ++$i);
			file_put_contents($filename, $image);
		}

		sleep(10);
		return 0;
	}

	function Download($siteid, $bookid, $chapterid)
	{
		global $http;
		global $sites;
		global $client;
		global $failedCount;

		$site = $sites[$siteid];
		$chapters = $site->GetChapter($bookid, $chapterid);
		if(False === $chapters){
			++$failedCount;
			print_r("task($bookid, $chapterid) failed[$failedCount], re-add it.\n");
			$workload = sprintf("%s,%d", $bookid, $chapterid);
			$client->doBackground('comic-bengou', $workload, $workload);
			return False;
		}

		$images = array();
		foreach($chapters as $chapter){
			$uri = $chapter["uri"];
			$referer = $chapter["referer"];
			$image = $http->get($uri, array('referer' => $referer));
			$images[] = $image;
		}
		return $images;
	}
?>
