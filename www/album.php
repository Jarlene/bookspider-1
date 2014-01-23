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
		sscanf($size, "%dx%d", $width, $height);

		$albums = zol_wallpaper_album($uri, $sort, $page, $orient);	
		foreach($albums as $key => $value){
			$img = array();
			$img2 = array();
			$images = zol_wallpaper_image($value);
			foreach($images as $image){
				$img[] = $image['dir'] . '/' . $size . '/' . $image['file'];
				if($width >= 1024){
					$img2[] = $image['dir'] . '/' . "960x600" . '/' . $image['file'];
				} else {
					$img2[] = $image['dir'] . '/' . "320x510" . '/' . $image['file'];
				}
			}

			$data[] = array(
				"name" => $key,
				"image" => $img,
				"image2" => $img2,
				"refer" => $uri,
				"size" => $width >= 1024 ? "960x600" : "320x510" //$size
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
