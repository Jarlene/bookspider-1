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
	function add_book($bookid, $author, $name, $icon, $summary, $catalog, $subcatalog)
	{
		$timestamp = date("Y-m-d H:i:s", time());
		$sql = sprintf('insert into books (bookid, author, name, icon, summary, catalog, subcatalog, updatetime) values ("%s", "%s", "%s", "%s", "%s", "%s", "%s", "%s")', $bookid, $author, $name, $icon, $summary, $catalog, $subcatalog, $timestamp);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function update_book($bookid, $author, $name, $icon, $summary, $catalog, $subcatalog)
	{
		$timestamp = date("Y-m-d H:i:s", time());
		$sql = sprintf('update books set author="%s", name="%s", icon="%s", summary="%s", catalog="%s", subcatalog="%s", updatetime="%s" where bookid="%s"', $author, $name, $icon, $summary, $catalog, $subcatalog, $timestamp, $bookid);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function get_book($bookid)
	{
		$sql = sprintf('select author, name, icon, summary, catalog, subcatalog from books where bookid="%s"', $bookid);
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
	
	function get_books()
	{
		$books = array();

		$sql = "select bookid, author, name, icon, summary, catalog, subcatalog from books";
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
	function add_chapter($bookid, $chapterid, $name, $uri)
	{
		$sql = sprintf('insert into chapters (bookid, chapterid, name, uri) values ("%s", %d, "%s", "%s")', $bookid, $chapterid, $name, $uri);
		$res = $this->db->query($sql);
		return False===$res ? -1 : 0;
	}

	function get_chapter($bookid, $chapterid)
	{
		$sql = sprintf('select name, uri from chapters where chapterid=%d', $chapterid);
		$res = $this->db->query($sql);
		if($res && $row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapter["uri"] = $row["uri"];
			$chapter["name"] = $row["name"];

			$res->free();
			return $chapter;
		}

		return False;
	}

	function get_chapters($bookid)
	{
		$chapters = array();

		$sql = sprintf('select chapterid, name, uri from chapters where bookid="%s"', $bookid);
		$res = $this->db->query($sql);
		if(False === $res)
			return $chapters;

		while($row = $res->fetch_assoc())
		{
			$chapter = array();
			$chapter["uri"] = $row["uri"];
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

		$sql = sprintf('select bookid, author, name, icon, summary, catalog, subcatalog from books where name like "*%s*"', $keyword);
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
	// member variable
	//----------------------------------------------------------------------------
	private $db;
}
?>
