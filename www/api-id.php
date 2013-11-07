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

	$id = php_reqvar("id", "0");
	
	$db = dbopen("joke");
	if($db->connect_errno)
	{
		echo "mysql error " . $db->connect->error;
		return;
	}

	$jokes = array();
	$tables = array("joke_text", "joke_image", "joke_18plus");
	for($i=0; $i<count($tables); $i++)
	{
		if(0 == $i)
			$sql = sprintf("select id,author,author_icon,datetime,content,approve,disapprove,comment from %s where id=%d", $tables[$i], $id);
		else
			$sql = sprintf("select id,author,author_icon,datetime,content,image,approve,disapprove,comment from %s where id=%d", $tables[$i], $id);

		$res = $db->query($sql);
		if (!$res)
		{
			echo "query nothing";
			continue;
		}

		while($row = $res->fetch_assoc())
		{
			$id = $row["id"];
			
			$joke = array();
			$joke["id"] = $row["id"];
			$joke["icon"] = $row["author_icon"];
			$joke["author"] = $row["author"];
			$joke["datetime"] = $row["datetime"];
			$joke["content"] = $row["content"];
			$joke["image"] = (0 != $i) ? $row["image"] : "";
			$joke["approve"] = $row["approve"];
			$joke["disapprove"] = $row["disapprove"];
			$joke["comment"] = $row["comment"];
			$jokes[] = $joke; // add book
		}
		$res->free();
	}

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
