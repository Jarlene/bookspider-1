<?php
	function autopage($db, $sort, $page)
	{
		//$res = $db->query("select count(0) from books");
		//$row = $res->fetch_assoc();
		//$count = ($row['count(0)']+49)/50;
		//$res->close();
		$count = (int)(dbcount($db, "books")+49)/50;
		
		if($page > 1)
		{
			$a = sprintf("<a class=\"page\" href=\"/book/books.php?sort=%s&page=%d\">%s</a>", $sort, $page-1, "Previous");
			echo $a;
		}
		
		$start = $page-5>0?$page-5:1;
		for($i=$start; $i<=$count && $i<$start+10; $i++)
		{
			if($i == $page)
				$a = sprintf("<a class=\"page\" href=\"/book/books.php?sort=%s&page=%d\" style=\"text-decoration: underline; color:red;\">%d</a>", $sort, $i, $i);
			else
				$a = sprintf("<a class=\"page\" href=\"/book/books.php?sort=%s&page=%d\">%d</a>", $sort, $i, $i);
			echo $a;
		}
		
		if($page < $count)
		{
			$a = sprintf("<a class=\"page\" href=\"/book/books.php?sort=%s&page=%d\">%s</a>", $sort, $page+1, "Next");
			echo $a;
		}
	}
?>
