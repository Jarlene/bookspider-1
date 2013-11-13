<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/api.inc");

	$page = php_reqvar("page", "1");
	$timestamp = php_reqvar("s", time());
	$limit = php_reqvar("limit", 50);

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = joke_query_hot($page, 1, 1, 1, $timestamp);
	echo json_encode($reply);
?>
