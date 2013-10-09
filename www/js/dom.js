// dom helper

function Find(xpath)
{
}

function FindId(id)
{
	var e = document.getElementById(id);
	return e;
}

function GetTextValue(id)
{
	var e = FindId(id);
	return e.value;
}

if(document.implementation.hasFeature("XPath", "3.0"))
{
	XMLDocument.prototype.selectNodes = function(xpath, xnode) {
		if(!xnode)
			xnode = this;

		var nodes = [];
		var nsResolver = this.createNSResolver(this.documentElement);
		var r = this.evaluate(xpath, xnode, nsResolver, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
		for(var i=0; i<r.snapshotLength; i++)
		{
			nodes[i] = r.snapshotItem(i);
		}
		return nodes;
	};

	Element.prototype.selectNodes = function selectNodes(xpath)
	{
		if(this.ownerDocument.selectNodes)
		{
			return this.ownerDocument.selectNodes(xpath, this);
		}
		else
		{
			throw "For XML Element Only";
		}
	}
}