<?php
	class AITINGWANG
	{
		public $cache = array(
							"catalog" => 2592000, // 30*24*60*60
							"book" => 604800,	  // 7*24*60*60
							"chapter" => 604800,
							"audio" => 604800,
							"search" => 604800
						);

		public $redirect = 1;
		public $useDelegate = 1;
		function GetName()
		{
			return "aitingwang";
		}

		function GetAudio($headers, $chapter, $html)
		{
			//$html = http_get($uri);
			//file_put_contents ("a10.html", $html);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			$uri1 = $xpath->get_attribute("//object/param[@name='url']", "value");

			$pos = strpos($uri1, "?");
			$vsid = substr($uri1, $pos+1);
			
			$referer = "http://www.aitingwang.com/" . $bookid;
			$referer = str_replace("-", "/ShowSoft.asp?SoftID=", $uri);
			
			$headers = array();
			$headers["Referer"] = $referer;
			$headers["Cookie"] = "virtualwall=$vsid";
			
			$reply = array();
			$reply["url"] = $uri1;
			$reply["headers"] = $headers;
			return $reply;
		}

		function GetChapters($bookid)
		{
// 			list($path, $id) = split("-", $bookid);
			$uri = "http://www.aitingwang.com/" . $bookid;
			$uri = str_replace("-", "/ShowSoft.asp?SoftID=", $uri);
			if ($this->useDelegate == 1)
				$html = http_proxy_get($uri, "10015884");
			else 
				$html = http_get($uri);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			
			$xpath = new XPath($html);
			$iconuri = $xpath->get_attribute("//table/tr/td/img[@class='pic2']", "src");
			$summary = $xpath->get_value("//table/tr[4]/td[@colspan='2']");
			
			
			$chapters = array();
			$chapters[0] = $this->__ParseChapters($html);
			
			

			$Result = array();
			$index = 0;
			{
				for($i = 0; $i < count($chapters); $i++)
				{
					foreach($chapters[$i] as $chapter)
					{
						
						$href = $chapter["uri"];
						
						$Result[$index] = array("name" => $chapter["name"], "uri" => $href);
						
						$index = $index + 1;
					}
				}
			}

			$data = array();
			$data["icon"] = $iconuri;
			$data["info"] = $summary;
			$data["chapter"] = $Result;
			return $data;
		}

		function GetBooks($uri)
		{
			$books = array();
			if(0 == strcmp($uri, 'http://www.aitingwang.com/bjjt/test/201406/11536.html')){ // update
				
				if ($this->useDelegate == 1)
					$html = http_proxy_get($uri, "10015884");
				else
					$html = http_get($uri);
				
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				
				$xpath = new XPath($html);
				$elements = $xpath->query("//td/table/tr/td/a[@class='shumin']");
				foreach ($elements as $element) 
				{
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					if(strlen($href) > 0 && strlen($book) > 0)
					{
						$bookid = strstr($href, "=");
						$bookid = substr($bookid, 1, strlen($bookid)-1);
						$host = parse_url($href);
						$href = $host["path"];
						$href = substr($href, 1, strlen($href)-1);
						$pos = stripos($href, "/");
						$href = substr($href, 0, $pos);
						$bookid = $href . "-" . $bookid;
						$books[$bookid] = $book;
					}
				}
				
			} else if(0 == strcmp($uri, 'http://www.aitingwang.com/bjjt/test/201405/11349.html')){ // top
				if ($this->useDelegate == 1)
					$html = http_proxy_get($uri, "10015884");
				else
					$html = http_get($uri);
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

				$xpath = new XPath($html);
				$elements = $xpath->query("//td/table/tr/td[@class='listbg']/a");
				foreach ($elements as $element) {
					$href = $element->getattribute('href');
					$book = $element->nodeValue;

					if(strlen($href) > 0 && strlen($book) > 0){
						$bookid = strstr($href, "=");
						$bookid = substr($bookid, 1, strlen($bookid)-1);
						$host = parse_url($href);
						$href = $host["path"];
						$href = substr($href, 1, strlen($href)-1);
						$pos = stripos($href, "/");
						$href = substr($href, 0, $pos);
						$bookid = $href . "-" . $bookid;
						$books[$bookid] = $book;
					}
				}
			}
			else if(0 == strcmp($uri, 'http://www.aitingwang.com/xs/test/201209/6514.html')){ // 郭德纲
				
// 				$book = "郭德纲对口相声大全";
// 				$bookid = "xs-6514";
// 				$books[$bookid] = $book;
				$books = $this->Search_real("郭德纲");
			}
			 else { // books
				if ($this->useDelegate == 1)
					$html = http_proxy_get($uri, "10015884");
				else
					$html = http_get($uri);
				$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

				$xpath = new XPath($html);
				$options = $xpath->query("//div[@class='page']/form/select/option");
				
				$totalNum = $xpath->get_value("//div[@class='showpage']/b[1]");
				$perNum = $xpath->get_value("//div[@class='showpage']/b[2]");
				
				$pageCount = floor($totalNum/$perNum);
				if (($totalNum%$perNum) != 0)
					$pageCount = $pageCount + 1;
				
				$pages = array();
				
				for ($i=2; $i<=$pageCount; $i=$i+1)
				{
					$pages[] = $uri . 'Index.asp?page=' . $i;
				}
				
				// page 1
				$result = array();
				$result[0] = $this->__ParseBooks($html);

				// other pages
				if(count($pages) > 0){
					if ($this->useDelegate == 1)
 						$http = new HttpMultipleProxy("proxy.cfg");
					else 
						$http = new HttpMultiple();
					$r = $http->get($pages, array($this, '_OnReadBook'), &$result, 60);
					if(0 != $r){
						// log error
					}
				}

 				if((count($pages)+1) == count($result))
				{
					for($i = 0; $i < count($result); $i++){
						foreach($result[$i] as $bid => $book){
							$books[$bid] = $book;
						}
					}
				}
			}

			$data = array();
			$data["icon"] = "";
			$data["book"] = $books;
			return $data;
		}

	function GetCatalog()
		{
			$uri = 'http://www.aitingwang.com/';
			
			if ($this->useDelegate == 1)
					$html = http_proxy_get($uri, "10015884");
				else
					$html = http_get($uri);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			$elements = $xpath->query("//div/table/tr/td/div/a");

			$subcatalogs = array();
			$subcatalogs["最近更新"] = 'http://www.aitingwang.com/bjjt/test/201406/11536.html';
			$subcatalogs["排行榜"] = 'http://www.aitingwang.com/bjjt/test/201405/11349.html';

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$subcatalog = $element->nodeValue;

				//$artist = mb_convert_encoding($artist, "gb2312", "UTF-8");
				//$artist = mb_convert_encoding($artist, "UTF-8", "gb2312");
				//$artist = iconv("GB18030", "UTF-8", $artist);
				if(strlen($href) > 0 && strlen($subcatalog) > 0 && ($href != 'http://www.aitingwang.com/Index.html'))
				{
					$subcatalogs[$subcatalog] = $href;
				}
			}

			$catalog = array();
			$catalog["小说"] = $subcatalogs;
			return $catalog;
		}

		function Search($keyword)
		{
			return array("book" => $this->Search_real($keyword));
		}
		
		function Search_real($keyword)
		{
			$uri = 'http://www.aitingwang.com/search.asp?ModuleName=soft';
			$data = 'Keyword=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . '&Submit=%CB%D1%CB%F7&Field=Title';
			$html = http_post($uri, $data);
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);

			$xpath = new XPath($html);
			
			$totalNum = $xpath->get_value("//div[@class='show_page']/b[1]");
			$perNum = $xpath->get_value("//div[@class='show_page']/b[2]");
			
			$pageCount = floor($totalNum/$perNum);
			if (($totalNum%$perNum) != 0)
				$pageCount = $pageCount + 1;
			
			$pages = array();
			
			for ($i=2; $i<=$pageCount; $i=$i+1)
			{
				$pages[] = 'http://www.aitingwang.com/Search.asp?ModuleName=soft&ChannelID=0&Field=Title&Keyword=' . urlencode(iconv("UTF-8", "gb2312", $keyword)) . "&ClassID=0&SpecialID=0&page=" . $i;
			}
			
			$books = array();

			
			// page 1
			$result = array();
			$result[0] = $this->__ParseSearch($html);

			// other pages
			if(count($pages) > 0){
				if ($this->useDelegate == 1)
					$http = new HttpMultipleProxy("proxy.cfg");
				else
					$http = new HttpMultiple();
				$r = $http->get($pages, array($this, '_OnReadSearch'), &$result, 60);
				if(0 != $r){
					// log error
				}
			}

// 			if(count($pages) == count($result))
			{
				for($i = 0; $i < count($result); $i++){
					foreach($result[$i] as $bid => $book){						
						$books[$bid] = $book;
					}
				}
			}

			return $books;
		}
		
		//---------------------------------------------------------------------------
		// private function
		//---------------------------------------------------------------------------
		function _OnReadBook($param, $i, $r, $header, $body)
		{
			if(0 != $r){
				//print_r("_OnReadBook $i error: $r\n");
				return -1;
			} else if(!stripos($body, "10015884")){
				// check html content integrity
				//print_r("_OnReadBook $i Integrity check error.\n");
				return -1;
			}

			$param[$i+1] = $this->__ParseBooks($body);
			//print_r("_OnReadBook $i: " . count($param[$i]) . "\n");
			return 0;
		}

		function _OnReadChapter($param, $i, $r, $header, $body)
		{
			if(0 != $r){
				//print_r("_OnReadBook $i error: $r\n");
				return -1;
			} else if(!stripos($body, "10015884")){
				// check html content integrity
				//print_r("_OnReadBook $i Integrity check error.\n");
				return -1;
			}
		
			$param[$i+1] = $this->__ParseChapters($body);
			//print_r("_OnReadBook $i: " . count($param[$i]) . "\n");
			return 0;
		}
		
		//---------------------------------------------------------------------------
		// private function
		//---------------------------------------------------------------------------
		function _OnReadSearch($param, $i, $r, $header, $body)
		{
			if(0 != $r){
				//print_r("_OnReadBook $i error: $r\n");
				return -1;
			} else if(!stripos($body, "10015884")){
				// check html content integrity
				//print_r("_OnReadBook $i Integrity check error.\n");
				return -1;
			}
		
			$param[$i+1] = $this->__ParseSearch($body);
			//print_r("_OnReadBook $i: " . count($param[$i]) . "\n");
			return 0;
		}
		
		private function __ParseBooks($html)
		{
			$books = array();
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
			
			$xpath = new XPath($html);
			$elements = $xpath->query("//td[@valign='middle']/table/tr/td/a[@class='shumin']");
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->nodeValue;

				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".html");
					$host = parse_url($href);
					$href = $host["path"];
					$href = substr($href, 1, strlen($href)-1);
					$pos = stripos($href, "/");
					$href = substr($href, 0, $pos);
					$bookid = $href . "-" . $bookid;
					$books[$bookid] = $book;
				}
			}

			return $books;
		}
		
		private function __ParseChapters($html)
		{
			$chapters = array();
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				
			$xpath = new XPath($html);
			//$elements = $xpath->query("//div[@class='border']/div/ul/li/a");
			$elements = $xpath->query("//table/tr[4]/td/a[@target='_blank']");

			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$chapter = $element->nodeValue;
			
				if(strlen($href) > 0 && strlen($chapter) > 0){
					$chapters[] = array("name" => $chapter, "uri" => $href);
//  					file_put_contents ("a.html", $href);
				}
			}
			
			
			return $chapters;
		}
		
		private function __ParseSearch($html)
		{
			$books = array();
			$html = str_replace("text/html; charset=gb2312", "text/html; charset=gb18030", $html);
				
			$xpath = new XPath($html);
			$elements = $xpath->query("//td/table/tr/td/a[@class='LinkSearchResult']");
			foreach ($elements as $element) {
				$href = $element->getattribute('href');
				$book = $element->nodeValue;
				if ($book == "播放列表")
					continue;
				
				if(strlen($href) > 0 && strlen($book) > 0){
					$bookid = basename($href, ".html");
					$host = parse_url($href);
					$href = $host["path"];
					$href = substr($href, 1, strlen($href)-1);
					$pos = stripos($href, "/");
					$href = substr($href, 0, $pos);
					$bookid = $href . "-" . $bookid;
					$books[$bookid] = $book;
				}
			}
		
			return $books;
		}
	}

//  require("php/dom.inc");
//  require("php/util.inc");
//  require("php/http.inc");
//  require("php/http-multiple.inc");
//  require("http-proxy.php");
//  require("http-multiple-proxy.php");
//  $obj = new AITINGWANG();
  
// print_r($obj->GetCatalog()); sleep(2);
//  print_r(count($obj->GetBooks("http://www.aitingwang.com/xs/test/201209/6514.html"))); sleep(2);
//	print_r(count($obj->GetBooks("http://www.aitingwang.com/kbxh/"))); sleep(2);
//      print_r($obj->GetChapters('kbxh-11537')); sleep(2); // http://www.77nt.com/DouFuXiaoShui/List_ID_8436.html
// print_r($obj->GetAudio('-xj803-test-201309-9309', '1', "http://www.aitingwang.com/xj803/ShowSoftDown.asp?UrlID=1&SoftID=9309")); sleep(2);
//  print_r($obj->Search("单田芳")); sleep(2);
?>
