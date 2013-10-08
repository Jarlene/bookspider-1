<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title>Book Search</title>
	<link rel="stylesheet" type="text/css" href="css/style.css" /> 
</head>
<body>
<div id="books">
<table align="center" width="98%">
	<tr style="background-color: #CCCCCC">
		<td width="50px" align="center"><a href="books.php?sort=bid">编号</a></td>
		<td width="75px" align="center"><a href="books.php?sort=category">类型</a></td>
		<td width="20%" align="center"><a href="books.php?sort=name">书名</a></td>
		<td width="15%" align="center"><a href="books.php?sort=author">作者</a></td>
		<td width="120px" align="center"><a href="books.php?sort=uptime">最后更新时间</a></td>
		<td width="50px" align="center"><a href="books.php?sort=vote">权值</a></td>
		<td width="30%" align="center"><a href="books.php?sort=chapter">最新章节</a></td>
		<td width="12px" align="center"><a href="/books.php?sort=mid">mid</a></td>
	</tr>
	
	<?php
	require("php/db.inc");
	require("php/util.inc");
	$db = dbopen("books");
	if($db->connect_errno)
		echo "mysql error " . $db->connect->error;
	
	$page = php_reqvar("page", "1");
	$type = php_reqvar("type", "");
	$book = php_reqvar("search", "");
	if(strlen($book) < 1)
		echo "search result is empty.";
	
	$page = $page < 1 ? 1 : $page;
	if(0==strcmp($type, "name"))
	{
		$sql = sprintf("select * from books where name like '%%%s%%' limit %d,50", mysql_real_escape_string($book), ($page-1)*50);
	}
	elseif(0 == strcmp($type, "author"))
	{
		$sql = sprintf("select * from books where author like '%%%s%%' limit %d,50", mysql_real_escape_string($book), ($page-1)*50);
	}
	elseif(0 == strcmp($type, "category"))
	{
		$sql = sprintf("select * from books where category like '%s' limit %d,50", mysql_real_escape_string($book), ($page-1)*50);
	}
	else
	{
		$sql = sprintf("select * from books where name like '%%%s%%' or author like '%%%s%%' limit %d,50", mysql_real_escape_string($book), mysql_real_escape_string($book), ($page-1)*50);
	}
	$res = $db->query($sql);
	while($row = $res->fetch_assoc())
	{
		echo "<tr>";
		echo sprintf("<td><a href=\"%s\">%s</a></td>", $row["uri"], $row["bid"]);
		echo sprintf("<td><a href=\"search.php?type=category&search=%s\">%s</a></td>", $row["category"], $row["category"]);
		echo sprintf("<td><a href=\"detail.php?book=%s\">%s</a></td>", $row["bid"], $row["name"]);
		echo sprintf("<td><a href=\"search.php?type=author&search=%s\">%s</a></td>", $row["author"], $row["author"]);
		echo "<td>" . $row["datetime"] . "</td>";
		echo "<td>" . $row["vote"] . "</td>";
		echo "<td>" . $row["chapter"] . "</td>";
		echo "<td>" . $row["mid"] . "</td>";
		echo "</tr>\n";
	}
	$res->free();
	?>
</table>
</div>
</body>
</html>
