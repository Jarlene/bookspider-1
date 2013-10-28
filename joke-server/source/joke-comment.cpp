#include "joke-comment.h"
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
	int len;
} FileHeader;

typedef struct
{
	int len;
	unsigned int id;
	time64_t datetime;
} ChunkHeader;

typedef std::map<unsigned int, Comment> Comments;
static Comments g_comments;

static int jokecomment_read(mmptr& ptr)
{
	StdCFile file("data/comment.bin", "rb");
	if(!file.IsOpened())
		return -1; // file don't exist

	void* content = file.Read();
	ptr.attach(content, 1);
	return 0;
}

static int jokecomment_write(mmptr& ptr)
{
	StdCFile file("data/comment.bin", "wb");
	if(!file.IsOpened())
		return -1; // file don't exist

	return file.Write(ptr.get(), ptr.size());
}

int jokecomment_init()
{
	mmptr ptr;
	int r = jokecomment_read(ptr);
	if(r < 0)
		return r;

	FileHeader* header = (FileHeader*)ptr.get();
	ChunkHeader* chunk = (ChunkHeader*)(header+1);
	while((char*)chunk < (char*)header+header->len)
	{
		Comment comment;
		comment.datetime = chunk->datetime;
		comment.comment.assign((const char*)(chunk+1), chunk->len);
		g_comments.insert(std::make_pair(chunk->id, comment));

		chunk = (ChunkHeader*)((char*)chunk + chunk->len + sizeof(ChunkHeader));
	}

	return g_comments.size();
}

int jokecomment_save()
{
	mmptr ptr(5*1024*1024);
	ptr.clear();

	FileHeader header2;
	ptr.append(&header2, sizeof(FileHeader));

	Comments::const_iterator it;
	for(it = g_comments.begin(); it != g_comments.end(); ++it)
	{
		const Comment& comment = it->second;

		ChunkHeader chunk;
		chunk.id = it->first;
		chunk.len = comment.comment.length();
		chunk.datetime = comment.datetime;
		ptr.append(&chunk, sizeof(chunk));
		ptr.append(comment.comment.c_str(), comment.comment.length());
	}

	FileHeader* header = (FileHeader*)ptr.get();
	header->len = ptr.size() + sizeof(FileHeader);
	header->crcc = 0;
	header->magic = 0xABCDEF09;
	header->ver = 1;

	return jokecomment_write(ptr);
}

int jokecomment_query(unsigned int id, time64_t datetime, std::string& comment)
{
	Comments::const_iterator it;
	it = g_comments.find(id);
	if(it == g_comments.end())
		return -1; // not found

	const Comment& item = it->second;
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
