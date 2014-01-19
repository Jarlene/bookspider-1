<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("zol_wallpaper.php");

	$page = php_reqvar("page", "1");
	$sort = php_reqvar("sort", "1");
	$device = php_reqvar("device", 'iphone5');
	$catalog = php_reqvar("catalog", 'all');

	$uri = zol_wallpaper_find($device, $catalog);

	$mc = new Memcached();
	$mc->addServer("localhost", 11211);

	$mckey = "album-" . $device . "-" . $catalog . "-" . $sort . "-" . $page;
	$data = $mc->get($mckey);

	if (!$data) {
		$data = array();
		$albums = zol_wallpaper_album($uri);
		foreach($albums as $key => $value){
			$images = zol_wallpaper_image($value);

			$data[] = array(
				"name" => $key,
				"image" => $images
			);
			
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
