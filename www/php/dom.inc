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
	if(!is_null($elements) && 1==$elements->length) 
		return $elements->item[0];
	return null;
}

?>
