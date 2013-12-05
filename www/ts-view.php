<?php
	require("php/db.inc");
	require("php/util.inc");

	$page = php_reqvar("page", "1");
	$limit = php_reqvar("limit", 50);
	
	$reply["code"] = 0;
	$reply["msg"] = "ok";
	$reply["feedback"] = ts_query($page, $limit);
	echo json_encode($reply);

	function ts_query($page, $limit)
	{
		$sql = sprintf("select * from ts order by datetime desc");
		
		// page
		$page = $page < 1 ? 1 : $page;
		$sql = $sql . sprintf(" limit %d,50", ($page-1)*50);

		// query
		$db = dbopen("feedback");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$res = $db->query($sql);
		
		$items = array();
		while($row = $res->fetch_assoc())
		{
			$item = array();
			$item["user"] = $row["user"];
			$item["datetime"] = $row["datetime"];
			$item["content"] = $row["content"];
			$item["contact"] = $row["contact"];
			$items[] = $item;
		}
		$res->free();
		return $items;
	}
?>
