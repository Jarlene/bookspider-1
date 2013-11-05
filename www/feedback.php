<?php
	require("php/db.inc");
	require("php/util.inc");

	$user = php_reqvar("u", "");
	$feedback = php_reqvar("c", "");
	joke_feedback($user, $feedback);
	
	function joke_feedback($user, $feedback)
	{
		// items
		$datetime = date_format(date_create(), 'Y-m-d H:i:s');
		$sql = sprintf("insert into feedback (user, datetime, content) values ('%s', '%s', '%s')", $user, $datetime, $feedback);
		
		// query
		$db = dbopen("joke");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$res = $db->query($sql);
	}
?>
