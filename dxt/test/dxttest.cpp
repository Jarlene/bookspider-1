#include "dxttest.h"
#include <assert.h>

static const char* html = 
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> \
<html xmlns=\"http://www.w3.org/1999/xhtml\"> \
<head >	\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /> \
</head>	\
<body>	\
<div>	\
</div>	\
<div id='div2'>	\
<h1 class='a'>header</h1>	\
<h1>header2</h1> \
<input type=\"text\" value=\"\" /> \
</div>	\
</body>	\
</html>";

static const char* xml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>	\
<xml>	\
<xsl:value-of select='/html/head/meta' />	\
<xsl:for-each select='/html/body/div[2]/h1'>	\
	<xsl:attr-of select='.' attr='a' /> \
</xsl:for-each>	\
</xml>";

int dxttest()
{
	char* result;
	int r = DXTransform(&result, html, 1, xml);
	assert(0==r);
	return r;
}
