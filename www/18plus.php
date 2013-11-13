<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/api.inc");

	$page = php_reqvar("page", "1");
	$timestamp = php_reqvar("s", time());
	$limit = php_reqvar("limit", 50);

	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["data"] = joke_query_comic($page, $timestamp);
	echo json_encode($reply);

	/// param[in] page page number, start from 1
	/// param[in] order 0-order by time, 1-order by approve
	/// param[in] range time range, 0-all, 1-8hours, 2-daily, 3-monthly, 4-yearly
	/// param[in] content 1-text only(default), 2-image only, 3-18+
	/// param[in] timestamp time measured in the number of seconds since the Unix Epoch (January 1 1970 00:00:00 GMT)
	function joke_query_comic($page, $timestamp)
	{
		$sql = "select id,title,text,image,datetime from joke_18plus";

		// condition
		$sqlcond = sprintf("datetime<='%s'", date("Y-m-d H:i:s", $timestamp));
		$sql = $sql . " where " . $sqlcond;

		// order
		$sql = $sql . " order by datetime desc";

		// page
		$page = $page < 1 ? 1 : $page;
		$sql = $sql . sprintf(" limit %d,50", ($page-1)*50);
		
		// query
		$db = dbopen("joke");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$res = $db->query($sql);

		$comics = array();
		while($row = $res->fetch_assoc())
		{
			$comic = array();
			$comic["id"] = $row["id"];
			$comic["title"] = $row["title"];
			$comic["content"] = $row["text"];
			$comic["images"] = split(",", $row["image"]);
			$comic["datetime"] = $row["datetime"];
			$comics[] = $comic; // add book
		}
		$res->free();
		return $comics;
	}
?>
