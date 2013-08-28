<?php
	require("php/db.inc");
	require("php/util.inc");

	$api = php_reqvar("api", "tip");
	$api(); // call function

	function search()
	{
		$book = php_reqvar("book", "");
		$page = php_reqvar("page", "1");
		$author = php_reqvar("author", "");
		if(strlen($book)<1 && strlen($author)<1)
		{
			echo "[]";
			return;
		}

		$db = dbopen("books");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$page = $page < 1 ? 1 : $page;
		if(strlen($author) > 0)
			$sql = sprintf("select * from books where author like '%%%s%%' limit %d,50", mysql_real_escape_string($author), ($page-1)*50);
		else
			$sql = sprintf("select * from books where name like '%%%s%%' or author like '%%%s%%' limit %d,50", mysql_real_escape_string($book), mysql_real_escape_string($book), ($page-1)*50);
		$res = $db->query($sql);

		$books = array();
		while($row = $res->fetch_assoc())
		{
			$book = array();
			$book["bid"] = $row["bid"];
			$book["uri"] = $row["uri"];
			$book["category"] = $row["category"];
			$book["name"] = $row["name"];
			$book["author"] = $row["author"];
			$book["chapter"] = $row["chapter"];
			$book["datetime"] = $row["datetime"];
			$books[] = $book; // add book
		}
		$res->free();

		echo json_encode($books);
	}

	function top()
	{
		$db = dbopen("books");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$page = php_reqvar("page", "1");
		$page = $page < 1 ? 1 : $page;
		$sql = sprintf("select * from books order by vote limit %d,50", ($page-1)*50);
		$res = $db->query($sql);

		$books = array();
		while($row = $res->fetch_assoc())
		{
			$book = array();
			$book["bid"] = $row["bid"];
			//$book["uri"] = $row["uri"];
			$book["category"] = $row["category"];
			$book["name"] = $row["name"];
			$book["author"] = $row["author"];
			$book["chapter"] = $row["chapter"];
			$book["datetime"] = $row["datetime"];
			$books[] = $book; // add book
		}
		$res->free();

		echo json_encode($books);
	}

	function query()
	{
		$bid = php_reqvar("bid", "");
		$page = php_reqvar("page", "1");
		if(strlen($bid) < 1)
		{
			echo "[]";
			return;
		}

		$db = dbopen("books");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$sqlconds = array();
		$bids = explode(",", $bid);
		foreach ($bids as $bid) {
			if($bid > 0 && $bid < 100000)
				$sqlconds[] =  "bid=" . "'" . $bid . "'";
		}
		$sqlcond = implode(" or ", $sqlconds);

		$page = $page < 1 ? 1 : $page;
		$sql = sprintf("select * from books where %s limit %d,50", $sqlcond, ($page-1)*50);
		
		$res = $db->query($sql);

		$books = array();
		while($row = $res->fetch_assoc())
		{
			$book = array();
			$book["bid"] = $row["bid"];
			//$book["uri"] = $row["uri"];
			//$book["category"] = $row["category"];
			//$book["name"] = $row["name"];
			//$book["author"] = $row["author"];
			$book["chapter"] = $row["chapter"];
			$book["datetime"] = $row["datetime"];
			$books[] = $book; // add book
		}
		$res->free();

		echo json_encode($books);
	}

	function tip()
	{
		echo "api.php?api=search/top";
	}
?>
