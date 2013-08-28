<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title>Book Detail</title>
	<link rel="stylesheet" type="text/css" href="css/style.css" /> 
</head>
<body>
<div id="books">
	<?php
	require("php/db.inc");
	require("php/util.inc");
	$db = dbopen("books");
	if($db->connect_errno)
		echo "mysql error " . $db->connect->error;
	
	$book = php_reqvar("book", "");
	if($book < 1)
		echo "book is is null." . $book;
	
	$sql = sprintf("select * from books where bid=%d", $book);
	$res = $db->query($sql);
	assert($res->num_rows == 1);
	while($row = $res->fetch_assoc())
	{
		echo "<table width=\"100%\">";
		echo "<tr style=\"background-color: #CCCCCC\">";
		echo "<td width=50px><a href='" . $row["uri"]. "'>" . $row["bid"] . "</a></td>";
		echo "<td width=75px>" . $row["category"] . "</td>";
		echo "<td width=20%>" . $row["name"] . "</td>";
		echo "<td width=15%>" . $row["author"] . "</td>";
		echo "<td width=120px>" . $row["datetime"] . "</td>";
		echo "<td width=50px>" . $row["vote"] . "</td>";
		echo "<td width=12px>" . $row["mid"] . "</td>";
		echo "</tr>\n";
		echo "</table>";
		
		echo "<table style=\"float:right;\" width=\"30%\">";
		echo "<tr style=\"background-color: #CCCCCC\"><td>" . $row["chapter"] . "</td></tr>";
		echo "<tr style=\"background-color: #CCCCCC\"><td>" . $row["chapter2"] . "</td></tr>";
		echo "<tr style=\"background-color: #CCCCCC\"><td>" . $row["chapter3"] . "</td></tr>";
		echo "</table>";
	}
	$res->close();
	?>
</div>

<?php
	$booksites = array("58xs"=>"58小说", "86zw"=>"八路中文", "luoqiu"=>"落秋小说");
	foreach($booksites as $booksite => $title)
	{
		$sql = sprintf("select bookuri, indexuri, datetime, chapter from %s, booklog where bid=%d and booklog.id=%s.mid", $booksite, $book, $booksite);
		if(!($res = $db->query($sql)))
			continue;
		
		assert($res->num_rows <= 1);
		if(1 == $res->num_rows)
		{
			$row = $res->fetch_assoc();
			echo "<div style='position: relative; left: 30px; clear:both;'><table width=80%><tr>";
			echo sprintf("<td width=50px>%s</td>", $title);
			echo sprintf("<td width=10%%><a href=\"%s\">book page</a></td>", $row["bookuri"]);
			echo sprintf("<td width=10%%><a href=\"%s\">book index</a></td>", $row["indexuri"]);
			echo sprintf("<td width=120px>%s</td>", $row["datetime"]);
			echo sprintf("<td width=30%%>%s</td>", $row["chapter"]);
			echo "</tr></table></div>\n";
		}
		$res->close();
	}
?>

</body>
</html>
