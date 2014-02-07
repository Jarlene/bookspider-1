<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("pingshu8.php");

	$book = php_reqvar("book", '');
	$artist = php_reqvar("artist", '');

	$mc = new Memcached();
	$mc->addServer("localhost", 11211);
	
	$mckey = "artists-" . $artist . "-works-" . $book;
	$chapters = $mc->get($mckey);
	if(!$chapters){
		$uri = "";
		$works = pingshu8_api_works($artist);
		foreach($works as $key => $href){
			if(0==strcmp($key, $book)){
				$uri = $href;
				break;
			}
		}

		if(strlen($uri) < 1){
			return array();
		}

		$data = array();
		$chapters = pingshu8_chapters($uri);
		foreach($chapters as $chapter => $href){
			$uri = pingshu8_audio($href);
			if(strlen($chapter) > 0 && strlen($uri) > 0){
				$data[] = array("name" => $chapter, "uri" => $uri);
			}
		}
		$mc->set($mckey, json_encode($data), 23*60*60);
	} else {
		$data = json_decode($chapters, True);
	}

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);
?>
