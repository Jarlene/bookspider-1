<?php
function http_get($uri)
{
	$header = array(
		'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
		'Accept-Encoding: gzip, deflate',
		'Accept-Language: en-US,en;q=0.5',
		'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:26.0) Gecko/20100101 Firefox/26.0'
	);

	// set URL and other appropriate options
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $uri);
	curl_setopt($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch, CURLOPT_ENCODING, '');
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, True);
	curl_setopt($ch, CURLOPT_HTTPHEADER, $header); 

	// grab URL and pass it to the browser
	$response = curl_exec($ch);
	//file_put_contents ("qiubai.html", $response);

	// close cURL resource, and free up system resources
	curl_close($ch);
	
	return $response;
}

function http_post($uri, $data)
{
	$header = array(
		'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
		'Accept-Encoding: gzip, deflate',
		'Accept-Language: en-US,en;q=0.5',
		'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:26.0) Gecko/20100101 Firefox/26.0'
	);

	// set URL and other appropriate options
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $uri);
	curl_setopt($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
	curl_setopt($ch, CURLOPT_ENCODING, '');
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, True);
	curl_setopt($ch, CURLOPT_HTTPHEADER, $header); 

	// grab URL and pass it to the browser
	$response = curl_exec($ch);
	//file_put_contents ("qiubai.html", $response);

	// close cURL resource, and free up system resources
	curl_close($ch);
	
	return $response;
}

function http_get_headers($uri)
{
	$header = array(
		'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
		'Accept-Encoding: gzip, deflate',
		'Accept-Language: en-US,en;q=0.5',
		'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:26.0) Gecko/20100101 Firefox/26.0'
	);

	// set URL and other appropriate options
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $uri);
	curl_setopt($ch, CURLOPT_HEADER, 1);
	curl_setopt($ch, CURLOPT_ENCODING, '');
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, True);
	curl_setopt($ch, CURLOPT_HTTPHEADER, $header); 

	// grab URL and pass it to the browser
	$response = curl_exec($ch);
	//file_put_contents ("qiubai.html", $response);
	
	$header_size = curl_getinfo($ch,CURLINFO_HEADER_SIZE);
	$result['header'] = substr($response, 0, $header_size);

	// close cURL resource, and free up system resources
	curl_close($ch);

	return $result['header'];
}
?>
