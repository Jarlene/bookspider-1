<?php
	require("php/db.inc");
	require("php/util.inc");

	$user = php_reqvar("u", "");
	$contact = php_reqvar("c", "");
	$feedback = php_reqvar("f", "");
	joke_feedback($user, $contact, $feedback);
	
	function joke_feedback($user, $contact, $feedback)
	{
		// items
		$datetime = date_format(date_create(), 'Y-m-d H:i:s');
		$sql = sprintf("insert into feedback (user, datetime, contact, content) values ('%s', '%s', '%s', '%s')", $user, $datetime, $contact, $feedback);
		
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
