<?php
	require("php/db.inc");
	require("php/sys.inc");
	require("php/dom.inc");
	require("php/http.inc");

	$db = dbopen("pingshu", "115.28.54.237");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	// proxies
	$http = new Http();
	$http->setcookie("/var/ysts8.cookie");
	$http->settimeout(120);
	$proxies = split(",", file_get_contents("proxy.cfg"));

	$task_count = 0;
	$ip = "";
	$servers = array("115.28.51.131", "115.28.54.237", "115.29.145.111", "112.126.69.201", "121.40.136.6", "175.195.249.184", "210.183.56.107");
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

	$paths = array("115.28.51.131" => "/ts2/ysts8/", "175.195.249.184" => "/home/ysts8/", "210.183.56.107" => "/home/ysts8");
	if(!array_key_exists($ip, $paths)){
		$basedir = "/ts/ysts8/";
	} else {
		$basedir = $paths[$ip];
	}

	print_r("IP: $ip\n");
	print_r("DIR: $basedir\n");

	$worker = new GearmanWorker();
	$worker->addServer('115.28.54.237', 4730);
	$worker->addFunction('DownloadYsts8', 'DoDownload');
	while ($worker->work());

	function DoDownload($job) {

		$args = $job->workload();
		list($bookid, $chapterid) = explode(",", $args);

		//$bookid = "4182";
		//$chapterid = 0005012703;
		return Action($bookid, intval($chapterid));
	}

	function db_set_chapter_uri($bookid, $chapterid, $uri)
	{
		global $db;
		$sql = sprintf('update ysts8 set uri="%s" where chapterid=%d', $uri, $chapterid);
		if(!$db->query($sql))
			print_r("DB set uri failed: " . $db->error);
		return $db->error;
	}

	function GetFrameSrc($bookid, $chapterid)
	{
		global $http;
		$uri = sprintf('http://www.ysts8.com/down_%d_%d_%d_%d.html', $bookid, (int)($chapterid/100000), (int)($chapterid/10000) % 10, (int)($chapterid % 10000));
		//$html = http_proxy_get($uri, "Ysjs/bot.js", 10);
		$html = $http->get($uri);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return "";

		$xpath = new XPath($html);
		$src = $xpath->get_attribute("//iframe[1]", "src");

		// "/play/flv.asp?url=http%3A%2F%2F180e%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB003%2Emp3&jiidx=/play%5F1836%5F49%5F1%5F4%2Ehtml&jiids=/play%5F1836%5F49%5F1%5F2%2Ehtml&id=1836&said=49"
		// "/play/flv_down.asp?wtid=http%3A%2F%2F180e%2Dd%2Eysts8%2Ecom%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB001%2Emp3&ctid=http%3A%2F%2F163e%2Dd%2Eysts8%2Ecom%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB001%2Emp3&title=%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB&ji=1&id=1836&said=49"
		return "http://www.ysts8.com" . $src;
	}

	function GetMP3File($bookid, $chapterid, $src, $ji, $said)
	{
		global $http;
		$referer = sprintf('http://www.ysts8.com/down_%d_%d_%d_%d.html', $bookid, $said, (int)($chapterid/10000) % 10, $ji);
		$html = $http->get($src, array('referer' => $referer));
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1){
			print_r("don't match suffix");
			return array("uri" => "", "referer" => "");
		}
	
		//file_put_contents ("ysts8.html", $html);

		// ?184465153683x1408069602x184759766437-172670378710548334
		// 6102986163870x1406030845x6103448776624-5be9cd2016294cd6a07a0a063876fdbc
		if(!preg_match('/([0-9]+x140[0-9]{7}x[0-9]+-[0-9a-fA-F\?\.]+)/', $html, $matches1)){
			print_r("don't match suffix");
			return array("uri" => "", "referer" => "");
		}

		$arr = explode("?", $src);
		$arr = explode("&", $arr[1]);
		$arr = explode("=", $arr[1]);
		$uri = urldecode($arr[1]);
		$uri = iconv("gb18030", "UTF-8", $uri);
		$uri = urlencode($uri);
		$uri = str_replace('%3A', ':', $uri);
		$uri = str_replace('%2F', '/', $uri);

		$suffix = $matches1[1];
		$result = "$uri?$suffix";
	
		//$html = $http->get("http://dxpsf-d.ysts8.com:8000/%E5%85%B6%E4%BB%96%E8%AF%84%E4%B9%A6/%E6%96%B0%E5%84%BF%E5%A5%B3%E8%8B%B1%E9%9B%84%E4%BC%A0/001.mp3", array('referer' => $uri));
		return array("uri" => $result, "referer" => $src);
	}

	function HttpGet($uri, $pattern, $headers)
	{
		global $http;
		global $proxies;
		static $idx = -1;

		if(count($proxies) > 0){
			$html = $http->get($uri, $headers);
		} else {
			if(-1 == $idx)
			{
				$idx = rand() % count($proxies);
				$http->setproxy($proxies[$idx]);
			}

			for($i = 0; $i < 5 && $i < count($proxies); $i++){
				$html = $http->get($uri, $headers);
				if(stripos($html, $pattern)){
					return $html;
				} else {
					unset($proxies[$idx]);
				}

				if(count($proxies) > 0){
					$idx = ($idx + 1) % count($proxies);
					print_r("[$idx] $proxy\n");
					$http->setproxy($proxies[$idx]);
				}
			}
		}

		return "";
	}

	function Download($bookid, $chapterid)
	{
		global $http;
		global $mdb;

		$src = GetFrameSrc($bookid, $chapterid);
		if(strlen($src) < 1) return False;

		$ji = $chapterid % 10000;
		$said = (int)($chapterid/100000);

		$mp3 = GetMP3File($bookid, $chapterid, $src, $ji, $said);
		if(strlen($mp3["uri"]) < 1) return False;
		print_r($mp3);

		$audio = $http->get($mp3["uri"], array('referer' => $mp3["referer"]));
		//$audio = HttpGet($mp3["uri"], array("referer: " . $mp3["referer"]));
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
		if(False === $audio)
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

		// write db
		db_set_chapter_uri($bookid, $chapterid, "$ip:$filename");

		// 
		sleep(60);
		return 0;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
