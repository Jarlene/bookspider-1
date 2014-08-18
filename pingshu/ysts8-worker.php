<?php
	require("php/db.inc");
	require("php/sys.inc");
	require("php/dom.inc");
	require("php/http.inc");

	// $db = dbopen("pingshu", "115.28.54.237");
	// if($db->connect_errno)
	// {
		// echo "mysql error " . $db->connect->error;
		// return;
	// }

	//$mdb = new Redis();
	//$mdb->connect('115.28.51.131', 6379);

	// proxies
	$http = new Http();
	$http->setcookie("/var/ysts8.cookie");
	$http->settimeout(120);
	$proxies = split(",", file_get_contents("proxy.cfg"));

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
	
	$ip = "127.0.0.2";
	if(strlen($ip) < 1)
	{
		print_r("server ip error.");
		return -1;
	}

	$paths = array("115.28.51.131" => "/ts2/ysts8/", "175.195.249.184" => "/home/ysts8/");
	if(!array_key_exists($ip, $paths)){
		$basedir = "/ts/ysts8/";
	} else {
		$basedir = $paths[$ip];
	}

	print_r("IP: $ip\n");
	print_r("DIR: $basedir\n");

	return Action("12073", 4620003);

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

	function GetAudioFile($bookid, $chapterid)
	{
		global $http;
		$uri = sprintf('http://www.ysts8.com/down_%d_%d_%d_%d.html', $bookid, (int)($chapterid/100000), (int)($chapterid/10000) % 10, (int)($chapterid % 10000));
		//$html = http_proxy_get($uri, "Ysjs/bot.js", 10);
		$html = $http->get($uri);
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1 || 200 != $http->get_code()) return "";

		$xpath = new XPath($html);
		$src = $xpath->get_attribute("//iframe[1]", "src");

		// "/play/flv.asp?url=http%3A%2F%2F180e%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB003%2Emp3&jiidx=/play%5F1836%5F49%5F1%5F4%2Ehtml&jiids=/play%5F1836%5F49%5F1%5F2%2Ehtml&id=1836&said=49"
		$arr = explode("&", $src);
		$arr = explode("?", $arr[0]);
		$arr = explode("=", $arr[1]);
		$uri = urldecode($arr[1]);
		$uri = iconv("gb18030", "UTF-8", $uri);

		$url = parse_url($uri);
		if(False === $url) return "";

		$file = $url["path"];
		return $file;
	}

	function GetMP3File($bookid, $chapterid, $title, $file, $ji, $said)
	{
		global $http;
		// http://www.ysts8.com/Yshtml/Ys13863.html
		// http://www.ysts8.com/play_13863_51_1_4.html
		// http://www.ysts8.com/down_1836_49_1_5.html
		// "/play/flv.asp?url=http%3A%2F%2F180e%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB003%2Emp3&jiidx=/play%5F1836%5F49%5F1%5F4%2Ehtml&jiids=/play%5F1836%5F49%5F1%5F2%2Ehtml&id=1836&said=49"
		// http://www.ysts8.com/play/flv_down.asp?wtid=http%3A%2F%2F180e%2Dd%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB005%2Emp3&ctid=http%3A%2F%2F163e%2Dd%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB005%2Emp3&title=%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB&ji=5&id=1836&said=49
		// $uri = sprintf('http://www.ysts8.com/play_%d_%d_%d_%d.html', $bookid, $chapterid/10000, ($chapterid/1000) % 10, $chapterid % 10000);
		//$referer = "referer: http://www.pingshu8.com/Play_Flash/js/Jplayer.swf";
		$servers = array("163e-d.ysx8.net:8000", "180e-d.ysx8.net:8000", "psf-d.ysts8.com:8000", "dxpsf-d.ysts8.com:8000");
		$server = $servers[2];
		$wtid = rawurlencode("http://$server$file");
		//$ctid = rawurlencode("http://$server$file");
		$wtid = str_replace('-', '%2D', $wtid);
		$wtid = str_replace('.', '%2E', $wtid);
		$ctid = str_replace('psf', 'dxpsf', $wtid);
		$title = rawurlencode($title);
		$uri = "http://www.ysts8.com/play/flv_down.asp?wtid=$wtid&ctid=$ctid&title=$title&ji=$ji&id=$bookid&said=$said";

		$referer = sprintf('http://www.ysts8.com/down_%d_%d_%d_%d.html', $bookid, $said, (int)($chapterid/10000) % 10, $ji);
		//$html = http_proxy_get($uri, ':8000', 10, "proxy.cfg", array($referer));
		$html = $http->get($uri, array('referer' => $referer));
		$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
		if(strlen($html) < 1) return "";
		file_put_contents ("ysts8.html", $html);

		// ?184465153683x1408069602x184759766437-172670378710548334
		// 6102986163870x1406030845x6103448776624-5be9cd2016294cd6a07a0a063876fdbc
		if(!preg_match('/([0-9]+x140[0-9]{7}x[0-9]+-[0-9a-fA-F\?\.]+)/', $html, $matches1)){
			print_r("don't match suffix");
			return array("uri" => "", "referer" => "");
		}

		$suffix = $matches1[1];
		$file = iconv("gb18030", "UTF-8", $file);
		$file = urlencode($file);
		$file = str_replace('%2F', '/', $file);
		$result = "http://dxpsf-d.ysts8.com:8000$file?$suffix";
	
		//$html = $http->get("http://dxpsf-d.ysts8.com:8000/%E5%85%B6%E4%BB%96%E8%AF%84%E4%B9%A6/%E6%96%B0%E5%84%BF%E5%A5%B3%E8%8B%B1%E9%9B%84%E4%BC%A0/001.mp3", array('referer' => $uri));
		return array("uri" => $result, "referer" => $uri);
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
		//global $mdb;

		//$file = $mdb->get("ts-file-$bookid");
		$file = null;
		if(!$file){
			$file = GetAudioFile($bookid, $chapterid);
			if(strlen($file) < 1) return "";
		}

		$ji = $chapterid % 10000;
		$said = (int)($chapterid/100000);
		$name = basename($file, ".mp3");
		if(preg_match('/([0-9]+)$/', $name, $matches)){
			if(2 == count($matches)){
				//$mdb->set("ts-file-$bookid", $file); // save file name
				$name2 = $matches[1];
				$dir = dirname($file);
				$partname = substr($name, 0, strlen($name)-strlen($name2));
				$file = sprintf("%s/%s%03d.mp3", $dir, $partname, $ji);
			}
		}

		if(strlen($file) < 1) return False;
		$file = iconv("UTF-8", "gb18030", $file);
		$mp3 = GetMP3File($bookid, $chapterid, '', $file, $ji, $said);
		if(strlen($mp3["uri"]) < 1) return False;

		$audio = $http->get($mp3["uri"], array('referer' => $mp3["referer"]));
		print_r($mp3);
		//$referer = "referer: http://www.ysts8.com/play/flv_down.asp?wtid=http%3A%2F%2F180e%2Dd%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB005%2Emp3&ctid=http%3A%2F%2F163e%2Dd%2Eysx8%2Enet%3A8000%2F%D0%FE%BB%C3%D0%A1%CB%B5%2F2009%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB%2F%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB005%2Emp3&title=%B7%B2%C8%CB%D0%DE%CF%C9%B4%AB&ji=5&id=1836&said=49";
		//$uri = "http://163e-d.ysx8.net:8000/%E7%8E%84%E5%B9%BB%E5%B0%8F%E8%AF%B4/2009/E5%87%A1%E4%BA%BA%E4%BF%AE%E4%BB%99%E4%BC%A0/%E5%87%A1%E4%BA%BA%E4%BF%AE%E4%BB%99%E4%BC%A0005.mp3";
		//$audio = http_get($uri, 120, "", array($referer, "Cookie: ASPSESSIONIDAQAQBSTB=EHHAMMJCAPBCABFGHKCBPDJN"));
		//$audio = http_get($mp3["uri"], 120, "", array("referer: " . $mp3["referer"]));
		//$audio = $http->get($mp3["uri"], array('referer' => $mp3["referer"]));
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
		return 0;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
