<?php
	require("php/sys.inc");
	require("db-pingshu.inc");

	$db = new DBPingShu("115.28.54.237");

	$ip = "";
	$cols = array("210.183.56.107" => "uri2", "175.195.249.184" => "uri");
	$servers = array("210.183.56.107", "175.195.249.184");
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

	$sites = array(//"pingshu8" => 1, 
					"ysts8" => 2);

	print_r("ip: $ip\n");
	print_r("action: remove files\n");
	$files2 = 0;
	list($files0, $files1) = ActionRemoveFile(); // remove pingshu8/ysts8 files

	print_r("action: add files\n");
	Action(); // add file to db

	print_r("------------------------------------------------------\n");
	print_r("clean uri: $files0, remove: $files1\n");
	print_r("add files: $files2\n");
	print_r("------------------------------------------------------\n");

	function Action()
	{
		global $sites;

		foreach($sites as $sitename => $siteid){
			$urls = DBListChapter($siteid);
			$dirs = array("/ts/$sitename", "/ts2/$sitename", "/home/$sitename");
			foreach($dirs as $dir){
				if(!is_dir($dir)){
					continue;
				}

				ListDir($dir, "DBAddFile", $siteid, $urls);
			}
		}

		return 0;
	}

	function DBListChapter($siteid)
	{
		global $db;
		global $ip;
		global $cols;

		$table = (1 == $siteid) ? "pingshu8" : "ysts8";
		$sql = sprintf('select bookid, chapterid, %s as file from %s', $cols[$ip], $table);
		$res = $db->exec($sql);
		if(False === $res)
			return False;

		$urls = array();
		while($row = $res->fetch_assoc())
		{
			$uri = $row["file"];
			$bookid = $row["bookid"];
			$chapterid = $row["chapterid"];
			$urls["$bookid:$chapterid"] = $uri;
		}

		$res->free();
		$res = null;
		return $urls;
	}
	
	function DBAddFile($siteid, $urls, $file)
	{
		global $ip;
		global $db;
		global $files2;
		$bookid = basename(dirname($file));
		$chapterid = basename($file, ".mp3");

		if(array_key_exists("$bookid:$chapterid", $urls)){
			$uri = $urls["$bookid:$chapterid"];

			$server = "";
			$path = "/";
			$parts = explode(":", $uri);
			if(count($parts) == 1){
				$server = "115.28.51.131";
				$path = $uri;
			} else {
				$server = $parts[0];
				$path = $parts[1];
			}

			if(0 == strcmp($ip, $server))
				return;
				
		}

		$uri = "$ip:$file";
		print_r("Add file[$bookid:$chapterid]: $uri\n");
		if(0 == strcmp("175.195.249.184", $ip))
			$r = $db->set_chapter_uri($siteid, $bookid, $chapterid, $uri);
		else
			$r = $db->set_chapter_uri2($siteid, $bookid, $chapterid, $uri);
		if(0 != $r)
			print_r("db add file ($uri) error:" . $db->get_error() . "\n");
		++$files2;
	}

	function ActionRemoveFile()
	{
		global $ip;
		global $db;
		global $cols;
		global $sites;

		$files0 = 0;
		$files1 = 0;
		foreach($sites as $sitename => $siteid){
			$chapters = $db->list_chapters($siteid);
			foreach($chapters as $chapter){
				$uri = $chapter[$cols[$ip]];
				$bookid = $chapter["bookid"];
				$chapterid = $chapter["chapterid"];

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
					print_r("db[$uri] file don't exist\n");
				} else {
					$fsize = filesize($path);
					if($fsize > 6*1024)
						continue;

					print_r("[$path] size: $fsize\n");
					print_r("remove[$path] => [$uri]\n");
					unlink($path);
					++$files1;
				}

				if(0 == strcmp("175.195.249.184", $ip))
					$r = $db->set_chapter_uri($siteid, $bookid, $chapterid, "");
				else
					$r = $db->set_chapter_uri2($siteid, $bookid, $chapterid, "");
				if(0 != $r)
					print_r("db clear ($siteid, $bookid, $chapterid) error:" . $db->get_error());
				++$files0;
			}
		}

		return array($files0, $files1);
	}

	function ListDir($dir, $callback, $param1, $param2)
	{
		$dh = opendir($dir);
		while( ($file = readdir($dh)) !== false){
			if(0 == strcmp(".", $file) || 0 == strcmp("..", $file)){
				continue;
			}

			$pathname = "$dir/$file";
			if(0 == strcmp("dir", filetype($pathname))){
				ListDir($pathname, $callback, $param1, $param2);
			} else {
				call_user_func($callback, $param1, $param2, $pathname);
			}
		}

		closedir($dh);
	}
?>
