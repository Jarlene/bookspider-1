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
	list($files0, $files1) = ActionRemoveFile(); // remove pingshu8/ysts8 files

	print_r("action: add files\n");
	$files2 = Action(); // add file to db

	print_r("------------------------------------------------------\n");
	print_r("clean uri: $files0, remove: $files1\n");
	print_r("add files: $files2\n");
	print_r("------------------------------------------------------\n");

	function Action()
	{
		global $db;
		global $ip;
		global $cols;
		global $sites;

		$num = 0;
		foreach($sites as $sitename => $siteid){
			$dirs = array("/ts/$sitename", "/ts2/$sitename", "/home/$sitename");
			foreach($dirs as $dir){
				$books = ListDir($dir);
				foreach($books as $book){
					$bookid = basename($book);
					$chapters = $db->get_chapters($siteid, $bookid);

					$files = ListDir($book);
					foreach($files as $file){
						$chapterid = basename($file, ".mp3");
						if(array_key_exists($chapterid, $chapters)){
							$chapter = $chapters[$chapterid];
							$uri = $chapter[$cols[$ip]];
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

							if(0 == strcmp($ip, $server)){
								continue;
							}
						}

						$uri = "$ip:$file";
						print_r("Add file[$bookid:$chapterid]: $uri\n");
						if(0 == strcmp("175.195.249.184", $ip))
							$r = $db->set_chapter_uri($siteid, $bookid, $chapterid, $uri);
						else
							$r = $db->set_chapter_uri2($siteid, $bookid, $chapterid, $uri);
						if(0 != $r)
							print_r("db add file ($uri) error:" . $db->get_error() . "\n");
						++$num;
					}
				}
			}
		}

		unset($chapters);
		unset($books);
		unset($files);
		return $num;
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
			$books = $db->get_books($siteid);
			foreach($books as $bookid => $book){
				$chapters = $db->get_chapters($siteid, $bookid);
				foreach($chapters as $chapterid => $chapter){
					$uri = $chapter[$cols[$ip]];

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
		}

		unset($chapters);
		unset($books);
		return array($files0, $files1);
	}

	function ListDir($dir)
	{
		$files = array();
		$dh = opendir($dir);
		if(False === $dh)
			return $files;

		while( ($file = readdir($dh)) !== false){
			if(0 == strcmp(".", $file) || 0 == strcmp("..", $file)){
				continue;
			}

			$pathname = "$dir/$file";
			$files[] = $pathname;
		}

		closedir($dh);
		return $files;
	}
?>
