<?php
function dom_parse($html)
{
	libxml_use_internal_errors(true);

	$doc = new DOMDocument();
	$doc->loadHTML($html);
	return $doc;
}

function xpath_query($doc, $path, $parent=null)
{
	$xpath = new DOMXpath($doc);
	if(is_null($parent)){
		$elements = $xpath->query($path);
	} else {
		$elements = $xpath->query($path, $parent);
	}
	return $elements;
}

function xpath_get($doc, $path, $parent=null)
{
	$elements = xpath_query($doc, $path, $parent);
	if(1 == $elements->length) 
		return $elements->item[0];
	return null;
}

class XPath
{
	function __construct($html)
	{
		libxml_use_internal_errors(true);

		$doc = new DOMDocument();
		$doc->loadHTML($html);
		$this->m_xpath = new DOMXpath($doc);
	}
	
	function query($path, $parent=null)
	{
		if(is_null($parent)){
			$elements = $this->m_xpath->query($path);
		} else {
			$elements = $this->m_xpath->query($path, $parent);
		}
		return $elements;
	}

	function get_value($path, $parent=null, $default=null)
	{
		$elements = $this->query($path, $parent);
		if(1 == $elements->length) 
			return $elements->item(0)->nodeValue;
		return $default;
	}

	function get_attribute($path, $attr, $parent=null, $default=null)
	{
		$elements = $this->query($path, $parent);
		if(1 == $elements->length) 
			return $elements->item(0)->getattribute($attr);
		return $default;
	}

	function get_text($path, $parent=null, $default=null)
	{
		$texts = array();
		$elements = $this->query($path, $parent);
		if(1 == $elements->length){
			foreach($elements->item(0)->childNodes as $node){
				if(XML_TEXT_NODE == $node->nodeType){
					$texts[] = $node->nodeValue;
				}
			}
			return join("\r\n", $texts);
		} else
			return $default;
		}
	}

	private $m_xpath;
};

?>
