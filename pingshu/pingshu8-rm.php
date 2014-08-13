<?php
	require("php/db.inc");

	$db = dbopen("pingshu", "115.28.54.237");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	$servers = array("115.28.51.131", "115.28.54.237", "115.29.145.111", "112.126.69.201", "121.40.136.6", "175.195.249.184");
	if(count($argv) < 2 || !in_array($argv[1], $servers))
	{
		print_r("please input server ip.");
		return -1;
	}
	
	if(count($argv) < 3)
	{
		print_r("please input action: RM/Tidy/Check\n");
		return -1;
	}
	
	$chapters = db_query();
	if(0 == strcmp("Check", $argv[2])){
		ActionCheck($argv[1], $chapters);
	} else if(0 == strcmp("RM", $argv[2]) || 0 == strcmp("Tidy", $argv[2])){
		Action($argv[1]);
	} else {
		print_r("unknown command\n");
		return -1;
	}

	function Action($ip)
	{
		global $argv;
		$dirs = array("/ts", "/ts2", "/home");
		foreach($dirs as $dir){
			if(!is_dir($dir)){
				continue;
			}

			ListDir($dir, "Action$argv[2]", $ip);
		}
		
		return 0;
	}

	function ActionCheck($ip, $chapters)
	{
		$files0 = 0;
		$files1 = 0;
		foreach($chapters as $key => $uri){
			list($bookid, $chapterid) = explode(":", $key);
			//print_r("$bookid/$chapterid: $uri\n");

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
				continue;

			if(!is_file($path)){
				print_r("db[$key - $uri] file don't exist\n");
			} else {
				$fsize = filesize($path);
				if($fsize > 2*1024)
					continue;

				print_r("[$path] size: $fsize\n");
				print_r("remove[$path] => [$uri]\n");
				unlink($path);
				++$files1;
			}

			db_set_chapter_uri($bookid, $chapterid, "");
			++$files0;
		}

		print_r("------------------------------------------------------\n");
		print_r("clean uri: $files0, remove: $files1\n");
		print_r("------------------------------------------------------\n");
	}
	
	function ActionTidy($ip, $file)
	{
		$bookid = basename(dirname($file));
		$chapterid = basename($file, ".mp3");

		global $chapters;
		if(array_key_exists("$bookid:$chapterid", $chapters)){
			$uri = $chapters["$bookid:$chapterid"];

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

			if(0 == strcmp($ip, $server) || 0 == strcmp("175.195.249.184", $server))
				return;
		}
		
		print_r("[$file] don't in database\n");
		$uri = "$ip:$file";
		print_r("Add file[$file] => $uri\n");
		db_set_chapter_uri($bookid, $chapterid, $uri);
	}

	function ActionRM($ip, $file)
	{
		$bookid = basename(dirname($file));
		$chapterid = basename($file, ".mp3");

		global $chapters;
		if(!array_key_exists("$bookid:$chapterid", $chapters)){
			print_r("file[$file] don't in database\n");
		} else {
			$uri = $chapters["$bookid:$chapterid"];
			
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

			if(0 != strcmp($ip, $server)){
				// file has moved
				if(0 != strcmp("175.195.249.184", $server)){
					// file don't move to korea server
					print_r("file[$file] map error: $uri\n");
				} else {
					print_r("remove[$file] => [$uri]\n");
					unlink($file);
				}
			}
		}	
	}

	function db_query()
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

			$chapters["$bookid:$chapterid"] = $uri;
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
	
	function ListDir($dir, $callback, $param)
	{
		$dh = opendir($dir);
		while( ($file = readdir($dh)) !== false){
			if(0 == strcmp(".", $file) || 0 == strcmp("..", $file)){
				continue;
			}

			$pathname = "$dir/$file";
			if(0 == strcmp("dir", filetype($pathname))){
				ListDir($pathname, $callback, $param);
			} else {
				call_user_func($callback, &$param, $pathname);
			}
		}

		closedir($dh);
	}
?>
