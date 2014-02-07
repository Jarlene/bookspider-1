<?php
	require("php/http.inc");
	require("php/dom.inc");
	require("php/util.inc");
	require("pingshu8.php");

	$data = array();

	$artists = pingshu8_api_artist();
	foreach($artists as $artist => $href){
		$data[] = $artist;
	}

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = $data;
	echo json_encode($reply);
?>
