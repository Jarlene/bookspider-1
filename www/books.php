<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Books</title>
<link rel="stylesheet" type="text/css" href="css/style.css" /> 
</head>
<body>
<div align="right" style="margin: 5px">
	<form method="post" action="search.php">
		search:
		<input type="text" name="search" />
		<input type="submit" value="Search" />
	</form>
</div>

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
		<td width="12px" align="center"><a href="books.php?sort=mid">mid</a></td>
	</tr>
	
<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/autopage.inc");
	$db = dbopen("books");
	if($db->connect_errno)
		echo "mysql error " . $db->connect->error;
	
	$page = php_reqvar("page", "1");
	$sort = php_reqvar("sort", "bid");
	$page = $page < 1 ? 1 : $page;
	
	$sql = sprintf("select * from books order by %s limit %d,50", mysql_real_escape_string($sort), ($page-1)*50);
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
	//$db->close();
?>
</table>
</div>

<div id="pages" align="right" style="margin:20px;">
<?php
	autopage($db, $sort, $page);
	$gotoform = sprintf("<form method=\"post\" action=\"books.php?sort=%s\">", $sort);
	echo $gotoform;
?>

	goto:
	<input type="text" name="page" style="vertical-align:text-center; width:30px"/>
	<?php
	echo "/" . (int)((dbcount($db, "books")+49)/50);
	?>
	<input type="submit" value="Go" />
</form>
</div>

<?php
$db->close();
?>
</body>
</html>
