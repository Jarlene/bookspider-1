#ifndef _book_spider_h_
#define _book_spider_h_

struct IBookSpider
{
	typedef int (*OnBook)(void* param, const char* book, const char* author, const char* uri, const char* chapter, const char* datetime);

	virtual ~IBookSpider(){}

	virtual int GetId() const = 0;
	virtual const char* GetName() const = 0;
	virtual int List(OnBook callback, void* param) = 0;
	virtual int Check(OnBook callback, void* param) = 0;
	virtual int Search(const char* book, const char* author, char *bookUri) = 0;
};

int SearchBook(const IBookSpider* spider, const char* uri, const char* req, const char* book, const char* author, char* bookUri);

int ListBook(const IBookSpider* spider, const char* uri, const char* req, IBookSpider::OnBook callback, void* param);

#endif /* !_book_spider_h_ */
