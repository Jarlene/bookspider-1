<?php
	/// param[in] page page number, start from 1
	/// param[in] order 0-order by time, 1-order by approve
	/// param[in] range time range, 0-all, 1-8hours, 2-daily, 3-monthly, 4-yearly
	/// param[in] content 0-image and text, 1-text only, 2-image only
	/// param[in] timestamp time measured in the number of seconds since the Unix Epoch (January 1 1970 00:00:00 GMT)
	function joke_query($page, $order, $range, $content, $timestamp)
	{
		// items
		$sql = "select id,icon,author,datetime,content,image,(approve+user_approve) as _approve, (disapprove+user_disapprove) as _disapprove,comment from joke";

		// condition
		if(1 == $content)
			$sqlcond = "image=''";
		else if(2 == $content)
			$sqlcond =  "image != ''";

		$timeend = date("Y-m-d H:i:s", $timestamp);
		if(1 == $range)
			$timefrom = date("Y-m-d H:i:s", $timestamp-8*60*60);
		else if(2 == $range)
			$timefrom = date("Y-m-d H:i:s", $timestamp-24*60*60);
		else if(3 == $range)
		{
			$timefrom = date_create($timeend);
			date_sub($timefrom, date_interval_create_from_date_string('1 month'));
			$timefrom = date_format($timefrom, "Y-m-d H:i:s");
		}
		else if(4 == $range)
		{
			$timefrom = date_create($timeend);
			date_sub($timefrom, date_interval_create_from_date_string('1 year'));
			$timefrom = date_format($timefrom, "Y-m-d H:i:s");
		}
		else
			$timefrom = $timeend;

		if(0 != strcmp($timefrom, $timeend))
		{
			if(strlen($sqlcond) > 0)
				$sqlcond = $sqlcond . " and ";
			$sqlcond = sprintf("(datetime>='%s' and datetime<='%s')", $timefrom, $timeend);
		}

		if(strlen($sqlcond) > 0)
			$sql = $sql . " where " . $sqlcond;

		// order
		if(1 == $order)
			$sql = $sql . " order by _approve desc";
		else
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

		$jokes = array();
		while($row = $res->fetch_assoc())
		{
			$id = $row["id"];
			
			$joke = array();
			if($id / 1000000000 == 1)
				$joke["comment_uri"] = sprintf("http://www.qiushibaike.com/article/%u", $id % 1000000000);
			else if($id / 1000000000 == 2)
				$joke["comment_uri"] = sprintf("http://budejie.com/detail.php?id=%u&nav=%d", $id % 1000000000, 1);
			else
				$joke["comment_uri"] = "";
			$joke["icon"] = $row["icon"];
			$joke["author"] = $row["author"];
			$joke["datetime"] = $row["datetime"];
			$joke["content"] = $row["content"];
			$joke["image"] = $row["image"];
			$joke["approve"] = $row["_approve"];
			$joke["disapprove"] = $row["_disapprove"];
			$joke["comment"] = $row["comment"];
			$jokes[] = $joke; // add book
		}
		$res->free();
		return $jokes;
	}
?>
