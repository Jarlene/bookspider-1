function JsonRequest()
{
	var v = false;

	try
	{
		v = new ActiveXObject("MSXML2.XMLHTTP");
	}
	catch(exception1)
	{
		try
		{
			v = new ActiveXObject("Microsoft.XMLHTTP");
		}
		catch(exception2)
		{
			v = false;
		}
	}

	if(!v && window.XMLHttpRequest)
	{
		v = new XMLHttpRequest();
	}

	return v;
}
