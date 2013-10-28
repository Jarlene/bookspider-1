<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Joke</title>
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
		<td width="120px" align="center"><a href="#">作者</a></td>
		<td width="150px" align="center"><a href="#">时间</a></td>
		<td width="50px" align="center"><a href="#">赞成</a></td>
		<td width="50px" align="center"><a href="#">反对</a></td>
		<td width="50px" align="center"><a href="#">评论</a></td>
		<td width="20%" align="center"><a href="#">笑话</a></td>
		<td width="120px" align="center"><a href="#">图片</a></td>
		<td width="120px" align="center"><a href="#">图标</a></td>
	</tr>

<?php
	require("php/db.inc");
	require("php/util.inc");
	require("php/autopage.inc");
	require("php/api.inc");
	
	$page = php_reqvar("page", "1");
	$content = php_reqvar("content", "1");
	$range = php_reqvar("range", "0");
	$order = php_reqvar("order", "0");
	$timestamp = php_reqvar("s", time());
	$limit = php_reqvar("limit", 50);
	
	$jokes = joke_query($page, $order, $range, $content, $timestamp);
	
	for($i=0; $i<count($jokes); $i++)
	{
		echo "<tr>";
		echo "<td>" . $jokes[$i]["author"] ."</td>";
		echo "<td>" . $jokes[$i]["datetime"] ."</td>";
		echo "<td>" . $jokes[$i]["approve"] ."</td>";
		echo "<td>" . $jokes[$i]["disapprove"] ."</td>";
		echo "<td><a href=\"" . $jokes[$i]["comment_uri"] . "\">" . $jokes[$i]["comment"] ."</a></td>";
		echo "<td>" . $jokes[$i]["content"] ."</td>";
		echo "<td>" . $jokes[$i]["image"] ."</td>";
		echo "<td>" . $jokes[$i]["icon"] ."</td>";
		echo "</tr>\n";
	}
	$res->free();
	//$db->close();
?>
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
	echo "/" . (int)((dbcount($db, "joke")+49)/50);
	?>
	<input type="submit" value="Go" />
</form>
</div>

<?php
$db->close();
?>
</body>
</html>
