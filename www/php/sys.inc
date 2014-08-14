<?php
/*
Array
(
    [eth0] => Array
        (
            [ip] => 10.161.77.91
            [mac] => 00:16:3E:00:3C:12
        )

    [eth1] => Array
        (
            [ip] => 115.29.145.111
            [mac] => 00:16:3E:00:01:2F
        )

)
*/
function get_network_interface()  
{  
	$ips   =   array();  
	$info   =   `/sbin/ifconfig   -a`;  
	$infos   =   explode("\n\n",   $info);  
	foreach   ($infos   as   $info)   {  
		$info   =   trim($info);  
		if   (substr($info,   0,   3)   ==   'eth')   {  
			$lines   =   explode("\n",   $info);  
			$interface   =   substr($lines[0],   0,   strpos($lines[0],   '   '));  
			$mac   =   substr($lines[0],   strlen($lines[0])-19);  
			preg_match('/addr:([0-9\.]+)/i',   $lines[1],   $matches);  
			$ip   =   $matches[1];  
			$ips[$interface]   =   array('ip'   =>   $ip,   'mac'   =>   $mac);  
		}
	}
	return   $ips;  
}
?>
