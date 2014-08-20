<?php
require_once("php/db.inc");

class DBPingShu
{
	function __construct($host="127.0.0.1", $usr="root", $pwd="")
	{
		$this->db = dbopen("pingshu", $host, $usr, $pwd);
		if($this->db->connect_errno)
		{
			echo "DBPingShu open error: " . $db->connect->error;
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
	function add_book($siteid, $bookid, $author, $name, $icon, $summary, $catalog, $subcatalog)
	{
		$timestamp = date("Y-m-d H:i:s", time());
		$sql = sprintf('insert into books (siteid, bookid, author, name, icon, summary, catalog, subcatalog, updatetime) values (%d, "%s", "%s", "%s", "%s", "%s", "%s", "%s", "%s")', $siteid, $bookid, $author, $name, $icon, $summary, $catalog, $subcatalog, $timestamp);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function update_book($siteid, $bookid, $author, $name, $icon, $summary, $catalog, $subcatalog)
	{
		$timestamp = date("Y-m-d H:i:s", time());
		$sql = sprintf('update books set author="%s", name="%s", icon="%s", summary="%s", catalog="%s", subcatalog="%s", updatetime="%s" where siteid=%d and bookid="%s"', $author, $name, $icon, $summary, $catalog, $subcatalog, $timestamp, $siteid, $bookid);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function get_book($siteid, $bookid)
	{
		$sql = sprintf('select author, name, icon, summary, catalog, subcatalog from books where siteid=%d and bookid="%s"', $siteid, $bookid);
		$res = $this->db->query($sql);
		if($res && $row = $res->fetch_assoc())
		{
			$book = array();
			$book["name"] = $row["name"];
			$book["icon"] = $row["icon"];
			$book["author"] = $row["author"];
			$book["summary"] = $row["summary"];
			$book["catalog"] = $row["catalog"];
			$book["subcatalog"] = $row["subcatalog"];

			$res->free();
			return $book;
		}
		return False;
	}

	function get_books($siteid)
	{
		$books = array();

		$sql = sprintf('select bookid, author, name, icon, summary, catalog, subcatalog from books where siteid=%d', $siteid);
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
			$book["catalog"] = $row["catalog"];
			$book["subcatalog"] = $row["subcatalog"];

			$bookid = $row["bookid"];
			$books[$bookid] = $book;
		}

		$res->free();
		return $books;
	}
	
	//----------------------------------------------------------------------------
	// chapter
	//----------------------------------------------------------------------------
	function add_chapter($siteid, $bookid, $chapterid, $name, $uri)
	{
		$table = (1 == $siteid) ? "pingshu8" : "ysts8";
		$sql = sprintf('insert into %s (bookid, chapterid, name, uri) values ("%s", %d, "%s", "%s")', $table, $bookid, $chapterid, $name, $uri);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function get_chapter($siteid, $bookid, $chapterid)
	{
		$table = (1 == $siteid) ? "pingshu8" : "ysts8";
		$sql = sprintf('select name, uri, uri2 from %s where chapterid=%d', $table, $chapterid);
		$res = $this->db->query($sql);
		if($res && $row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapter["uri"] = $row["uri"];
			$chapter["uri2"] = $row["uri2"];
			$chapter["name"] = $row["name"];

			$res->free();
			return $chapter;
		}

		return False;
	}

	function get_chapters($siteid, $bookid)
	{
		$chapters = array();

		$table = (1 == $siteid) ? "pingshu8" : "ysts8";
		$sql = sprintf('select chapterid, name, uri, uri2 from %s where bookid="%s"', $table, $bookid);
		$res = $this->db->query($sql);
		if(False === $res)
			return $chapters;

		while($row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapter["uri"] = $row["uri"];
			$chapter["uri2"] = $row["uri2"];
			$chapter["name"] = $row["name"];

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
