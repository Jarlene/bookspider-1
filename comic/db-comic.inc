<?php
require_once("php/db.inc");

class DBComic
{
	function __construct($host="127.0.0.1", $usr="root", $pwd="")
	{
		$this->db = dbopen("comic", $host, $usr, $pwd);
		if($this->db->connect_errno)
		{
			echo "DBComic open error: " . $db->connect->error;
			return;
		}
	}

	function __destruct()
	{
		dbclose($this->db);
	}

	function get_error()
	{
		return $this->db->error;
	}

	//----------------------------------------------------------------------------
	// book
	//----------------------------------------------------------------------------
	function add_book($siteid, $bookid, $author, $name, $icon, $summary, $region, $catalog, $tags, $date)
	{
		$sql = sprintf('insert into books (siteid, bookid, author, name, icon, summary, region, catalog, tags, date) values (%d, "%s", "%s", "%s", "%s", "%s", "%s", "%s", "%s", "%s")', $siteid, $bookid, $author, $name, $icon, $summary, $region, $catalog, $tags, $date);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function update_book($siteid, $bookid, $author, $name, $icon, $summary, $region, $catalog, $tags, $date)
	{
		$timestamp = date("Y-m-d H:i:s", time());
		$sql = sprintf('update books set author="%s", name="%s", icon="%s", summary="%s", region="%s", catalog="%s", tags="%s", date="%s" where siteid=%d and bookid="%s"', $author, $name, $icon, $summary, $region, $catalog, $tags, $date, $siteid, $bookid);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function get_book($siteid, $bookid)
	{
		$sql = sprintf('select author, name, icon, summary, region, catalog, tags, date from books where siteid=%d and bookid="%s"', $siteid, $bookid);
		$res = $this->db->query($sql);
		if($res && $row = $res->fetch_assoc())
		{
			$book = array();
			$book["name"] = $row["name"];
			$book["icon"] = $row["icon"];
			$book["author"] = $row["author"];
			$book["summary"] = $row["summary"];
			$book["region"] = $row["region"];
			$book["catalog"] = $row["catalog"];
			$book["tags"] = $row["tags"];
			$book["date"] = $row["date"];

			$res->free();
			return $book;
		}
		return False;
	}

	function get_books($siteid)
	{
		$books = array();

		$sql = sprintf('select bookid, author, name, icon, summary, region, catalog, tags, date from books where siteid=%d', $siteid);
		$res = $this->db->query($sql);
		if(False === $res)
			return $books;

		while($row = $res->fetch_assoc())
		{
			$book = array();
			$book["name"] = $row["name"];
			$book["icon"] = $row["icon"];
			$book["author"] = $row["author"];
			$book["summary"] = $row["summary"];
			$book["region"] = $row["region"];
			$book["catalog"] = $row["catalog"];
			$book["tags"] = $row["tags"];
			$book["date"] = $row["date"];

			$bookid = $row["bookid"];
			$books[$bookid] = $book;
		}

		$res->free();
		return $books;
	}
	
	//----------------------------------------------------------------------------
	// chapter
	//----------------------------------------------------------------------------
	function add_chapter($siteid, $bookid, $chapterid, $name, $section)
	{
		$table = (1 == $siteid) ? "bengou" : "imanhua";
		$sql = sprintf('insert into %s (bookid, chapterid, name, section) values ("%s", %d, "%s", "%s")', $table, $bookid, $chapterid, $name, $section);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function set_chapter_uri($siteid, $bookid, $chapterid, $uri)
	{
		$table = (1 == $siteid) ? "bengou" : "imanhua";
		$sql = sprintf('update %s set uri="%s" where bookid="%s" and chapterid=%d', $table, $uri, $bookid, $chapterid);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}
	
	function set_chapter_uri2($siteid, $bookid, $chapterid, $uri)
	{
		$table = (1 == $siteid) ? "bengou" : "imanhua";
		$sql = sprintf('update %s set uri2="%s" where bookid="%s" and chapterid=%d', $table, $uri, $bookid, $chapterid);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function get_chapter($siteid, $bookid, $chapterid)
	{
		$table = (1 == $siteid) ? "bengou" : "imanhua";
		$sql = sprintf('select name, section from %s where bookid="%s" and chapterid=%d', $table, $bookid, $chapterid);
		$res = $this->db->query($sql);
		if($res && $row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapter["name"] = $row["name"];
			$chapter["section"] = $row["section"];

			$res->free();
			return $chapter;
		}

		return False;
	}

	function get_chapters($siteid, $bookid)
	{
		$chapters = array();

		$table = (1 == $siteid) ? "bengou" : "imanhua";
		$sql = sprintf('select chapterid, name, section from %s where bookid="%s"', $table, $bookid);
		$res = $this->db->query($sql);
		if(False === $res)
			return $chapters;

		while($row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapter["name"] = $row["name"];
			$chapter["section"] = $row["section"];

			$chapterid = $row["chapterid"];
			$chapters[$chapterid] = $chapter;
		}

		$res->free();
		return $chapters;
	}

	//----------------------------------------------------------------------------
	// search
	//----------------------------------------------------------------------------
	function search($keyword)
	{
		$books = array();

		$sql = sprintf('select siteid, bookid, author, name, icon, summary, catalog, subcatalog from books where siteid=1 and name like "%%%s%%"', $keyword);
		$res = $this->db->query($sql);
		if(False === $res)
			return $books;

		while($row = $res->fetch_assoc())
		{
			$book = array();
			$book["siteid"] = $row["siteid"];
			$book["name"] = $row["name"];
			$book["icon"] = $row["icon"];
			$book["author"] = $row["author"];
			$book["summary"] = $row["summary"];
			$book["catalog"] = $row["catalog"];
			$book["subcatalog"] = $row["subcatalog"];

			$bookid = $row["bookid"];
			$books[$bookid] = $book;
		}

		$res->free();
		return $books;
	}

	//----------------------------------------------------------------------------
	// member variable
	//----------------------------------------------------------------------------
	private $db;
}
?>
