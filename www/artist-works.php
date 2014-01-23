<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("pingshu8.php");

	$artist = php_reqvar("artist", '');

	$data = array();
	$works = pingshu8_api_works($artist);
	foreach($works as $key => $href){
		$data[] = $key;
	}

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);
?>
