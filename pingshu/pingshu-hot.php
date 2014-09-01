<?php
require_once("db-pingshu.inc");

$dbhost = "127.0.0.1";
$db = new DBPingShu($dbhost);

$mdb = new Redis();
$mdb->connect('127.0.0.1', 6379);
$keys = $mdb->keys("ts-server-hot-pingshu8-book-*");
foreach($keys as $key){
	$bookid = substr($key, strlen("ts-server-hot-pingshu8-book-"));
	if(count(explode("_", $bookid)) > 1)
		$siteid = 1;
	else
		$siteid = 2;

	$value = $mdb->get($key);
//	print_r("item($siteid, $bookid) $key: $value\n");
	$sql = sprintf('update books set hot=%d where siteid=%d and bookid="%s"', (int)$value, $siteid, $bookid);
	$db->exec($sql);
}
?>
