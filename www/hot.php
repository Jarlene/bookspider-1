<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/api.inc");

	$page = php_reqvar("page", "1");
	$content = php_reqvar("content", "1");
	$timestamp = php_reqvar("s", time());
	$limit = php_reqvar("limit", 50);

	$jokes = joke_query_hot($page, 1, 1, $content, $timestamp);
	echo json_encode($jokes);
?>
