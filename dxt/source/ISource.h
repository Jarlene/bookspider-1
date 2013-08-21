#ifndef _ISource_h_
#define _ISource_h_

struct ISource
{
	virtual ~ISource(){}

	// path: /html/body/div[3]/div[class="style",title="title"]/a
	virtual bool Foreach(const char* path, const char* match) = 0;
	virtual bool Foreach(const char* path) = 0;
	virtual bool ForeachNext() = 0;

	virtual bool GetName(const char* path, std::string& name) const = 0;

	virtual bool GetValue(const char* path, const char* match, std::string& value) const = 0;
	virtual bool GetValue(const char* path, std::string& value) const = 0;

	virtual bool GetAttr(const char* path, const char* name, std::string& value) const = 0;
};

#endif /* !_ISource_h_ */
