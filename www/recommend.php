<?php
	app_recommend();

	function app_recommend()
	{
		$apps = array();
		for($i=0; $i<10; $i++)
		{
			$app = array();
			$app["name"] = "yaomeier";
			$app["icon"] = sprintf("/app/app%d/icon.png", $i);
			$app["uri"] = sprintf("https://play.google.com/store/apps/details?id=com.yaomeier");
			$app["description"] = "";
			$app["range"] = 5;
			
			$apps[] = $app;
		}
		
		$reply["apps"] = $apps;
		echo json_encode($reply);
	}
?>
