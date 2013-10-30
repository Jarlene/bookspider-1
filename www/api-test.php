<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Joke</title>
<link rel="stylesheet" type="text/css" href="css/style.css" /> 
<script src="js/xml.js" type="text/javascript"></script>
</head>
<body>
<div id="books">
<table align="center" width="98%">
	<tr style="background-color: #CCCCCC">
		<td width="100px" align="center"><a href="#">作者</a></td>
		<td width="160px" align="center"><a href="#">时间</a></td>
		<td width="40px" align="center"><a href="#">赞成</a></td>
		<td width="40px" align="center"><a href="#">反对</a></td>
		<td width="40px" align="center"><a href="#">评论</a></td>
		<td align="center"><a href="#">笑话</a></td>
		<td width="300px" align="center"><a href="#">图片</a></td>
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
		echo "<td>";
		if(strlen($jokes[$i]["icon"]) > 0)
			echo "<image src=\"" . $jokes[$i]["icon"] . "\" />";
		echo $jokes[$i]["author"] ."</td>";
		echo "<td>" . $jokes[$i]["datetime"] ."</td>";
		echo "<td>" . $jokes[$i]["approve"] ."</td>";
		echo "<td>" . $jokes[$i]["disapprove"] ."</td>";
		echo "<td><a href=\"/joke/comment.html?id=" . $jokes[$i]["id"] . "\">" . $jokes[$i]["comment"] ."</a></td>";
		echo "<td>" . $jokes[$i]["content"] ."</td>";
		echo "<td>" . $jokes[$i]["image"] ."</td>";
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
