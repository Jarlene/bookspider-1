<?php
	function php_getvar($name, $default)
	{
		return isset($_GET[$name]) ? $_GET[$name] : $default;
	}
	
	function php_postvar($name, $default)
	{
		return isset($_POST[$name]) ? $_GET[$name] : $default;
	}
	
	function php_reqvar($name, $default)
	{
		return isset($_REQUEST[$name]) ? $_REQUEST[$name] : $default;
	}
?>
