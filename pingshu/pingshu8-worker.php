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

	function GetAudioUri($bookid, $chapterid)
	{
		// 'http://211.99.142.60:81/1Q2W3E4R5T6Y7U8I9O0P1Z2X3C4V5B/down1.pingshu8.com:8000/3/ps/%B5%A5%CC%EF%B7%BC_%D1%EE%E7%DB%B4%AB(40%BB%D8)/%B5%A5%CC%EF%B7%BC_%D1%EE%E7%DB%B4%AB_01.mp3';
		$uri = "http://www.pingshu8.com/bzmtv_Inc/download.asp?fid=$chapterid";
		// $uri = "http://www.pingshu8.com/path321_" . $chapterid . ".html";
		// $referer = "Referer: http://www.pingshu8.com/play_" . $chapterid . ".html";

		// $html = http_proxy_post($uri, "", ":8000", 10, "proxy.cfg", array($referer));
		// if(0 == strlen($html)){
		// 	$uri = "http://www.pingshu8.com/play_" . $chapterid . ".html";
		// 	$html = http_proxy_get($uri, "luckyzz@163.com", 10, "proxy.cfg", array($referer));
		// 	// var urlpath = encodeURI("http://play0.pingshu8.com:8000/0/ps/田连元_水浒人物传(297回)/田连元_水浒人物传_002.flv?1181593355719x1407335492x1182119998091-5757be2f0bbc@123abcd805521ccbbb833b9@123abcd0e");
		// 	if(!preg_match('/encodeURI\(\"(.+)\"\)/', $html, $matches)){
		// 		return "";
		// 	}

		// 	if(2 != count($matches)){
		// 		return "";
		// 	}

		// 	$uri = $matches[1];
		// } else {
		// 	$obj = json_decode($html);
		// 	$uri = $obj->{"urlpath"};
		// }

		// $uri = str_replace("@123abcd", "9", $uri);
		// $uri = str_replace(".flv", ".mp3", $uri);
		return $uri;
	}

	function Download($uri)
	{
		//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
		$referer = "referer: http://www.pingshu8.com/Play_Flash/js/Jplayer.swf";
		$audio = http_get($uri, 60, "", array($referer));
		// $proxy = "proxy.cfg";
		// $random = time() * 6;
		// $proxies = split(",", file_get_contents($proxy));
		// for($i  = 0; $i < 5 && $i < count($proxies); $i++){
		// 	$proxy = $proxies[($i + $random) % count($proxies)];
		// 	print_r("$proxy\n");
		// 	$audio = http_get($uri, 120, $proxy, array($referer));
		// 	//if(0 === stripos($audio, "ID3")){
		// 		return $audio;
		// 	//}
		// }

		return $audio;
	}

	function Action($bookid, $chapterid)
	{
		print_r("Action: $bookid, $chapterid\n");
		//$uri = "http://www.pingshu8.com/play_" . $chapterid . ".html";
		//$obj = new CPingShu8();
		//$uri = $obj->GetAudio($bookid, $chapterid, $uri);
		$uri = GetAudioUri($bookid, $chapterid);
		if(strlen($uri) < 1)
		{
			print_r("Action($bookid, $chapterid) GetAudioUri failed.\n");
			return -1;
		}

		$audio = Download($uri);
		if(0 == strlen($audio))
		{
			print_r("Download($uri) failed.\n");
			return -1;
		}

		// write file
		if(!is_dir("/ts2/pingshu8/$bookid")){
			mkdir("/ts2/pingshu8/$bookid", 777, true);
		}

		$filename = sprintf("/ts2/pingshu8/%s/%d.mp3", $bookid, $chapterid);
		file_put_contents($filename, $audio);

		// write db
		db_set_chapter_uri($bookid, $chapterid, "115.28.51.131:/$filename");
		return 0;
	}

	//$uri = "http://www.englishbaby.com/lessons/download_21st_mp3/6659/87156b073e8631d4763d377207bfe779dc7d351bf19b7c019e4cafea1609bd66";
	//$audio = Download($uri);
	//file_put_contents("a.mp3", $audio);
?>
