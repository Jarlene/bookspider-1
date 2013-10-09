// xml helper

function HttpRequest()
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

function XmlParser(xml)
{
	if(window.DOMParser)
	{
		parser = new DOMParser();
		doc = parser.parseFromString(xml, "text/xml");
	}
	else // Internet Explorer
	{
		doc = new ActiveXObject("Microsoft.XMLDOM");
		doc.async = false;
		doc.loadXML(xml);
	}
	return doc;
}