<?php
	function dbopen($database)
	{
		$host = "localhost";
		$usr = "root";
		$pwd = "";
		$db = new mysqli($host, $usr, $pwd, $database);
		return $db;
	}
	
	function dbclose($db)
	{
		$db->close();
	}
	
	function dbcount($db, $table)
	{
		$sql = sprintf("select count(0) from %s", mysql_real_escape_string($table));
		$res = $db->query($sql);
		$row = $res->fetch_assoc();
		return $row['count(0)'];
	}
?>
