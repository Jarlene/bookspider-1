<?php
	require("php/db.inc");
	require("php/util.inc");

	$api = php_reqvar("api", "top");
	$api(); // call function

	function top()
	{
		$db = dbopen("joke");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$page = php_reqvar("page", "1");
		$page = $page < 1 ? 1 : $page;
		$sql = sprintf("select * from joke limit %d,50", ($page-1)*50);
		$res = $db->query($sql);

		$jokes = array();
		while($row = $res->fetch_assoc())
		{
			$joke = array();
			$joke["author"] = $row["author"];
			$joke["datetime"] = $row["datetime"];
			$joke["content"] = $row["content"];
			$joke["image"] = $row["image"];
			$joke["approve"] = $row["approve"];
			$joke["disapprove"] = $row["disapprove"];
			$jokes[] = $joke; // add book
		}
		$res->free();

		echo json_encode($jokes);
	}

	function tip()
	{
		echo "api.php?api=search/top";
	}
?>
