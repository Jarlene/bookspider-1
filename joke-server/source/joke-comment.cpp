#include "joke-comment.h"
#include "sys/sync.hpp"
#include "systimer.h"
#include "StdCFile.h"
#include "mmptr.h"
#include <map>

#define VALID_TIME (60*60*1000)

typedef struct
{
	int hot;
	time64_t datetime;
	std::string comment;
} TComment;

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
	unsigned int id;
	time64_t datetime;
} FChunk;

typedef std::map<unsigned int, TComment> Comments;
static Comments s_comments;
static ThreadLocker s_locker;
static systimer_t s_timer;

int Inflate(const void* ptr, size_t len, mmptr& result);
int Deflate(const void* ptr, size_t len, mmptr& result);

static int jokecomment_read(mmptr& ptr)
{
	StdCFile file("data/comment.bin", "rb");
	if(!file.IsOpened())
		return -1; // file don't exist

	long fsize = file.GetFileSize();
	void* content = file.Read(fsize);
	assert((size_t)fsize == sizeof(FHeader) + ((FHeader*)ptr.get())->len);

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

static void jokecomment_recycle()
{
	time64_t lt = time64_now();
	Comments::iterator it, it0;

	AutoThreadLocker locker(s_locker);
	it = s_comments.begin();
	while( it != s_comments.end() )
	{
		it0 = it++;
		TComment& comment = it0->second;
		if(comment.datetime + VALID_TIME < lt)
			s_comments.erase(it0);
	}
}

static void jokecomment_timer(systimer_t /*timer*/, void* /*param*/)
{
	jokecomment_recycle();
	jokecomment_save();
}

int jokecomment_init()
{
	mmptr ptr;
	int r = jokecomment_read(ptr);
	if(r < 0)
		return r;

	AutoThreadLocker locker(s_locker);
	FHeader* header = (FHeader*)ptr.get();
	FChunk* chunk = (FChunk*)(header+1);
	while((char*)chunk < (char*)(header+1)+header->len)
	{
		TComment comment;
		comment.datetime = chunk->datetime;
		comment.comment.assign((const char*)(chunk+1), chunk->len);
		s_comments.insert(std::make_pair(chunk->id, comment));

		chunk = (FChunk*)((char*)(chunk+1) + chunk->len);
	}

	systimer_start(&s_timer, VALID_TIME, jokecomment_timer, NULL);
	return s_comments.size();
}

int jokecomment_save()
{
	mmptr ptr(5*1024*1024);
	ptr.clear();

	FHeader header2;
	ptr.append(&header2, sizeof(FHeader));

	{
		Comments::const_iterator it;
		AutoThreadLocker locker(s_locker);
		for(it = s_comments.begin(); it != s_comments.end(); ++it)
		{
			const TComment& comment = it->second;

			FChunk chunk;
			chunk.id = it->first;
			chunk.len = comment.comment.length();
			chunk.datetime = comment.datetime;
			ptr.append(&chunk, sizeof(chunk));
			ptr.append(comment.comment.c_str(), comment.comment.length());
		}
	}

	FHeader* header = (FHeader*)ptr.get();
	header->len = ptr.size();
	header->crcc = 0;
	header->magic = 0xABCDEF09;
	header->ver = 1;

	return jokecomment_write(ptr);
}

int jokecomment_query(unsigned int id, time64_t& datetime, std::string& comment)
{
	Comments::iterator it;
	AutoThreadLocker locker(s_locker);
	it = s_comments.find(id);
	if(it == s_comments.end())
		return -1; // not found

	TComment& item = it->second;
	item.hot += 1; // visit times

	datetime = item.datetime;
	comment = item.comment;
	return 0;
}

int jokecomment_insert(unsigned int id, time64_t datetime, const std::string& comment)
{
	Comments::iterator it;
	AutoThreadLocker locker(s_locker);
	it = s_comments.find(id);
	if(it == s_comments.end())
	{
		TComment item;
		item.hot = 1;
		item.datetime = datetime;
		item.comment = comment;
		s_comments.insert(std::make_pair(id, item));
	}
	else
	{
		TComment& item = it->second;
		item.datetime = datetime;
		item.comment = comment;
	}

	return 0;
}
