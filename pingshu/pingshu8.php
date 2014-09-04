<?php	
require_once("db-pingshu.inc");

class CPingShu8
{
	public $cache = array(
					"catalog" => 864000, // 10*24*60*60
					"book" => 43200,   // 24*60*60
					"chapter" => 43200, // 24*60*60
					"audio" => 0,
					"search" => 0 // 24*60*60
				);

	public $redirect = 0;
	public static $siteid = 1;

	function GetName()
	{
		return "pingshu8";
	}

	function GetAudio($bookid, $chapter, $uri)
	{
		global $headers;

		//list($play, $chapterid) = explode("_", basename($uri, ".html"));
		$chapterid = $uri;

		if(count(explode("_", $bookid)) > 1)
			$siteid = self::$siteid;
		else
			$siteid = 2;

		// update user access
		$this->UpdateBookHot($siteid, $bookid);

		// query db
		$chapter = $this->db->get_chapter($siteid, $bookid, $chapterid);
		if(False === $chapter)
			return "";

		$uri = $chapter["uri"];
		$uri2 = $chapter["uri2"];
		if (strlen($uri2) > 0)
		{
			if(0==strlen($uri))   //只有uri2,走uri2
				$uri = $uri2;
			else if(0 < strlen($uri))  //  uri1和uri2同时存在，x/10走uri2
			{
				if (0 == strcmp(substr($uri, 0, 14), '115.29.145.111'))  // uri在111服务器上, 走uri2
					$uri = $uri2;
				else
				{
					$rand = rand(1,10);
					if ($rand > 10)
						$uri = $uri2;
				}
			}
		}

		$server = "";
		$path = "/";
		$urls = explode(":", $uri);
		if(count($urls) == 1){
			$server = "115.28.51.131";
			$path = $uri;
		} else {
			$server = $urls[0];
			$path = $urls[1];
		}

		if(strlen($path) < 1)
			return "";

		$uri = $path;
		$uri = str_replace("//home/pingshu8", "/1", $uri);
		$uri = str_replace("/home/pingshu8", "/1", $uri);
		$uri = str_replace("//ts/pingshu8", "/2", $uri);
		$uri = str_replace("/ts/pingshu8", "/2", $uri);
		$uri = str_replace("//ts2/pingshu8", "/3", $uri);
		$uri = str_replace("/ts2/pingshu8", "/3", $uri);
		$uri = str_replace("//home/ysts8", "/11", $uri);
		$uri = str_replace("/home/ysts8", "/11", $uri);
		$uri = str_replace("//ts/ysts8", "/12", $uri);
		$uri = str_replace("/ts/ysts8", "/12", $uri);
		$uri = str_replace("//ts2/ysts8", "/13", $uri);
		$uri = str_replace("/ts2/ysts8", "/13", $uri);
		return "http://$server$uri";
	}

	//----------------------------------------------------------------------------
	// GetChapters
	//----------------------------------------------------------------------------
	function GetChapters($bookid)
	{
		if(count(explode("_", $bookid)) > 1)
			$siteid = self::$siteid;
		else
			$siteid = 2;

		$book = $this->db->get_book($siteid, $bookid);
		$chapters = $this->db->get_chapters($siteid, $bookid);
		if(False == $book || 0 == count($chapters))
			return False;

		ksort($chapters);
		$data = array();
		$data["icon"] = $book["icon"];
		$data["info"] = $book["summary"];
		$data["count"] = count($chapters);
		$data["chapter"] = array();
		$data["catalog"] = $book["catalog"];
		$data["subcatalog"] = $book["subcatalog"];
		foreach($chapters as $id => $value){
			$data["chapter"][] = array("name" => $value["name"], "uri" => $id);
		}

		return $data;
	}

	//----------------------------------------------------------------------------
	// GetBooks
	//----------------------------------------------------------------------------
	function GetBooks($uri)
	{
		$books = array();
		$n1 = strpos($uri, "最近更新");
		$n2 = strpos($uri, "排行榜");
		if($n1 > 0){
			$catalog = substr($uri, 0, $n1);
			$sql = sprintf('select bookid,name from books where siteid=%d and catalog="%s" order by updatetime limit 0,100', self::$siteid, $catalog);
		} else if($n2 > 0){
			$catalog = substr($uri, 0, $n2);
			$sql = sprintf('select bookid,name from books where siteid=%d and catalog="%s" order by hot limit 0,100', self::$siteid, $catalog);
		} else {
			$subcatalog = $uri;
			$sql = sprintf('select bookid,name from books where siteid=%d and subcatalog="%s"', self::$siteid, $subcatalog);
		}

		$res = $this->db->exec($sql);
		while($row = $res->fetch_assoc())
		{
			$bookid = $row["bookid"];
			$name = $row["name"];
			$books[$bookid] = $name;
		}

		$res->free();
		$res = null;
		
		$data = array();
		$data["icon"] = "";
		$data["book"] = $books;
		return $data;
	}

	function GetCatalog()
	{
		$catalogs = array();
		$catalogs["评书"] = array("评书最近更新" => "在线评书最近更新", "评书排行榜" => "在线评书排行榜");
		$catalogs["小说"] = array("小说最近更新" => "有声小说最近更新", "小说排行榜" => "有声小说排行榜");
		$catalogs["相声小品"] = array("相声小品最近更新" => "相声小品最近更新", "相声小品排行榜" => "相声小品排行榜");
		$catalogs["金庸全集"] = array("金庸作品最近更新" => "金庸作品全集最近更新", "金庸作品排行榜" => "金庸作品全集排行榜");
		$catalogs["综艺娱乐"] = array("综艺娱乐最近更新" => "综艺节目最近更新", "综艺娱乐排行榜" => "综艺节目排行榜");

		$sql = sprintf('select distinct(catalog),subcatalog from books where siteid=%d', self::$siteid);
		$res = $this->db->exec($sql);
		while($row = $res->fetch_assoc())
		{
			$catalog = $row["catalog"];
			$subcatalog = $row["subcatalog"];
			if(strlen($catalog) < 1 || strlen($subcatalog) < 1 || 0==strcmp("金庸有声小说", $catalog))
				continue;

			$catalog = str_replace("在线评书", "评书", $catalog);
			$catalog = str_replace("有声小说", "小说", $catalog);
			$catalog = str_replace("有声节目", "综艺娱乐", $catalog);
			$catalog = str_replace("金庸作品全集", "金庸全集", $catalog);
			if(!array_key_exists($catalog, $catalogs)){
				//$catalogs[$catalog] = array("$catalog最近更新" => "$catalog最近更新", "$catalog排行榜" => "$catalog排行榜");
				continue;
			}

			$catalogs[$catalog][$subcatalog] = $subcatalog;
		}

		$res->free();
		$res = null;

//		$catalog["评书"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_1.Htm', 'http://www.pingshu8.com/top/pingshu.htm', '评书排行榜');
//		$catalog["相声小品"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_2.Htm', 'http://www.pingshu8.com/top/xiangsheng.htm', '相声小品排行榜');
//		$catalog["小说"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_3.Htm', 'http://www.pingshu8.com/top/yousheng.htm', '小说排行榜');
//		$catalog["金庸全集"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_4.Htm', 'http://www.pingshu8.com/Special/Msp_218.Htm', '金庸作品排行榜');
//		$catalog["综艺娱乐"] = $this->__GetSubcatalog('http://www.pingshu8.com/Music/bzmtv_5.Htm', 'http://www.pingshu8.com/top/zongyi.htm', '综艺娱乐排行榜');
		return $catalogs;
		print_r($catalogs);
	}

	function Search($keyword)
	{
		$sql = sprintf('select bookid, name from books where siteid=%d and name like "%%%s%%" or author like "%%%s%%"', self::$siteid, $keyword, $keyword);

		$res = $this->db->exec($sql);
		if(False === $res)
			return array("catalog" => array(), "book" => array());

		$books = array();
		while($row = $res->fetch_assoc())
		{
			$bookid = $row["bookid"];
			$books[$bookid] = $row["name"];
		}

		$res->free();
		$res = null;

		return array("catalog" => array(), "book" => $books);
	}

	//---------------------------------------------------------------------------
	// user function
	//---------------------------------------------------------------------------
	function UpdateBookHot($siteid, $bookid)
	{
		global $mdb;

		$hotkey = "ts-server-hot-" . $this->GetName() . "-book-" . $bookid;
		if(!$mdb->exists($hotkey)){
			$sql = sprintf('select hot from books where siteid=%d and bookid="%s"', $siteid, $bookid);
			$res = $this->db->exec($sql);
			if($res && $row = $res->fetch_assoc()){
				$mdb->set($hotkey, (int)$row["hot"]);
			}
		}

		$value = $mdb->incr($hotkey);
	}

	//----------------------------------------------------------------------------
	// constructor
	//----------------------------------------------------------------------------
	function __construct()
	{
		$this->db = new DBPingShu("115.28.54.237");
	}

	private $db;
}

// $obj = new CPingShu8();
// print_r($obj->GetCatalog()); sleep(2);
// print_r($obj->GetBooks("金庸作品全集")); sleep(2);
// print_r($obj->GetBooks("单田芳")); sleep(2);
// print_r($obj->GetBooks("奇志 大兵")); sleep(2);
// print_r($obj->GetBooks("最近更新-在线评书")); sleep(2);
// print_r($obj->GetBooks("排行榜-在线评书")); sleep(2);
// print_r($obj->GetChapters('33_239_1')); sleep(2);
// print_r($obj->GetAudio('7_208_1', '1', "http://www.pingshu8.com/play_19632.html")); sleep(2);
// print_r($obj->Search("单田芳")); sleep(2);
// print_r($obj->__GetAudio("http://www.pingshu8.com/play_27123.html"));
// print_r($obj->GetAudio("201_3751_1", "1", "http://www.pingshu8.com/play_161404.html"));
?>
