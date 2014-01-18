<?php
function dom_parse($html)
{
	libxml_use_internal_errors(true);

	$doc = new DOMDocument();
	$doc->loadHTML($html);
	return $doc;
}

function xpath_query($doc, $path)
{
	$xpath = new DOMXpath($doc);
	$elements = $xpath->query($path);
	return $elements;
}
?>
