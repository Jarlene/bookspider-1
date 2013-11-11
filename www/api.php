<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/api.inc");

	$page = php_reqvar("page", "1");
	$content = php_reqvar("content", "1");
	$range = php_reqvar("range", "0");
	$order = php_reqvar("order", "0");
	$timestamp = php_reqvar("s", time());
	$limit = php_reqvar("limit", 50);
	
	if(1==$order && 1==$range)
		$jokes = joke_query_hot($page, $order, $range, $content, $timestamp);
	else
		$jokes = joke_query($page, $order, $range, $content, $timestamp);
	echo json_encode($jokes);
?>
