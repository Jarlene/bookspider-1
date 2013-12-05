<?php
	require("php/db.inc");
	require("php/util.inc");

	$user = php_reqvar("u", "");
	$contact = php_reqvar("c", "");
	$feedback = php_reqvar("f", "");
	ts_feedback($user, $contact, $feedback);
	
	function ts_feedback($user, $contact, $feedback)
	{
		// items
		$datetime = date_format(date_create(), 'Y-m-d H:i:s');
		$sql = sprintf("insert into ts (user, datetime, contact, content) values ('%s', '%s', '%s', '%s')", $user, $datetime, $contact, $feedback);
		
		// query
		$db = dbopen("feedback");
		if($db->connect_errno)
		{
			echo "mysql error " . $db->connect->error;
			return;
		}

		$res = $db->query($sql);
		echo "thanks";
	}
?>
