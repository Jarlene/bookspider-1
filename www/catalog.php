<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("zol_wallpaper.php");

	$device = php_reqvar("device", 'iphone5');
	$catalogs = zol_wallpaper_device($device);

	$data = array();
	foreach($catalogs as $i){
		$subcatalog = array();
		foreach($i->subcatalog as $key => $value){
			$subcatalog[] = $key;
		}

		$catalog = array();
		$catalog["catalog"] = $i->name;
		$catalog["subcatalog"] = $subcatalog;
		$data[] = $catalog;
	}

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);
?>
