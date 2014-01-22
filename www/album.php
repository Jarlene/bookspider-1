<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("zol_wallpaper.php");

	$page = php_reqvar("page", "1");
	$sort = php_reqvar("order", "");
	$orient = php_reqvar("orient", 'seascape');
	$device = php_reqvar("device", 'iphone5');
	$catalog = php_reqvar("catalog", 'all');

	$uri = zol_wallpaper_find($device, $catalog);
	if(strlen($uri) < 1){
		$reply["code"] = 0;
		$reply["msg"] = "ok";
		$reply["data"] = array();
		echo json_encode($reply);
	}

	$mc = new Memcached();
	$mc->addServer("localhost", 11211);

	if(0 != strcmp("hot", $sort)){
		$sort = "";
	}

	$mckey = "album-" . $device . "-" . $catalog . "-" . $sort . "-" . $page . "-" . $orient;
//	$mc->delete($mckey);
	$data = $mc->get($mckey);

	if (!$data) {
		$data = array();
		$size = zol_wallpaper_resolution($device);

		$albums = zol_wallpaper_album($uri, $sort, $page, $orient);	
		foreach($albums as $key => $value){
			$img = array();
			$images = zol_wallpaper_image($value);
			foreach($images as $image){
				$img[] = $image['dir'] . '/' . $size . '/' . $image['file'];
			}

			$data[] = array(
				"name" => $key,
				"image" => $img,
				"refer" => $uri,
				"size" => $size
			);
		}

		if(count($data) > 0){
			$mc->set($mckey, json_encode($data), 23*60*60);
		}
	} else {
		$data = json_decode($data);
	}

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);
?>
