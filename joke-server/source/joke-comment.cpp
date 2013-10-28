#include "joke-comment.h"
#include "os-timer.h"
#include "StdCFile.h"
#include "mmptr.h"
#include <map>

typedef struct
{
	time64_t datetime;
	std::string comment;
} Comment;

typedef struct
{
	int magic;
	int crcc;
	int ver;
	int len; // content length(don't include FHeader size)
} FHeader;

typedef struct
{
	int len; // comment length(don't include FChunk size)
	int hot;
	unsigned int id;
	time64_t datetime;
} FChunk;

typedef std::map<unsigned int, Comment> Comments;
static Comments g_comments;

int Inflate(const void* ptr, size_t len, mmptr& result);
int Deflate(const void* ptr, size_t len, mmptr& result);

static int jokecomment_read(mmptr& ptr)
{
	StdCFile file("data/comment.bin", "rb");
	if(!file.IsOpened())
		return -1; // file don't exist

	long fsize = file.GetFileSize();
	void* content = file.Read(fsize);
	assert(fsize == sizeof(FHeader) + ((FHeader*)ptr.get())->len);

	int r = Deflate(content, fsize, ptr); // uncompress
	free(content);
	return r;
}

static int jokecomment_write(const mmptr& ptr)
{
	StdCFile file("data/comment.bin", "wb");
	if(!file.IsOpened())
		return -1; // file don't exist

	mmptr result;
	int r = Inflate(ptr.get(), ptr.size(), result);
	if(0 != r)
		return r; // compress failed

	return file.Write(result.get(), result.size());
}

int jokecomment_init()
{
	mmptr ptr;
	int r = jokecomment_read(ptr);
	if(r < 0)
		return r;

	FHeader* header = (FHeader*)ptr.get();
	FChunk* chunk = (FChunk*)(header+1);
	while((char*)chunk < (char*)(header+1)+header->len)
	{
		Comment comment;
		comment.datetime = chunk->datetime;
		comment.comment.assign((const char*)(chunk+1), chunk->len);
		g_comments.insert(std::make_pair(chunk->id, comment));

		chunk = (FChunk*)((char*)(chunk+1) + chunk->len);
	}

	return g_comments.size();
}

int jokecomment_save()
{
	mmptr ptr(5*1024*1024);
	ptr.clear();

	FHeader header2;
	ptr.append(&header2, sizeof(FHeader));

	Comments::const_iterator it;
	for(it = g_comments.begin(); it != g_comments.end(); ++it)
	{
		const Comment& comment = it->second;

		FChunk chunk;
		chunk.id = it->first;
		chunk.len = comment.comment.length();
		chunk.datetime = comment.datetime;
		ptr.append(&chunk, sizeof(chunk));
		ptr.append(comment.comment.c_str(), comment.comment.length());
	}

	FHeader* header = (FHeader*)ptr.get();
	header->len = ptr.size();
	header->crcc = 0;
	header->magic = 0xABCDEF09;
	header->ver = 1;

	return jokecomment_write(ptr);
}

int jokecomment_query(unsigned int id, time64_t datetime, std::string& comment)
{
	Comments::iterator it;
	it = g_comments.find(id);
	if(it == g_comments.end())
		return -1; // not found

	Comment& item = it->second;
	item.hot += 1; // visit times

	datetime = item.datetime;
	comment = item.comment;
	return 0;
}

int jokecomment_insert(unsigned int id, time64_t datetime, const std::string& comment)
{
	Comments::iterator it;
	it = g_comments.find(id);
	if(it == g_comments.end())
	{
		Comment item;
		item.host = 1;
		item.datetime = datetime;
		item.comment = comment;
		g_comments.insert(std::make_pair(id, item));
	}
	else
	{
		Comment& item = it->second;
		item.datetime = datetime;
		item.comment = comment;
	}

	return 0;
}
