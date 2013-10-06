#include "zlib.h"
#include "mmptr.h"

#define CHUNK 16384

int Deflate(const void* ptr, size_t len, mmptr& result)
{
	int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

	result.clear();

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    
    flush = Z_FINISH;
	strm.avail_in = len;
    strm.next_in = (z_const Bytef *)ptr;

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = deflate(&strm, flush);    /* no bad return value */
        assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        have = CHUNK - strm.avail_out;
		if(result.append(out, have))
		{
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);     /* all input will be used */

    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

int Inflate(const void* ptr, size_t len, mmptr& result)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char out[CHUNK];

	result.clear();

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	//ret = inflateInit(&strm);
	ret = inflateInit2(&strm, 16+MAX_WBITS);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = len;
		/* Check for source > 64K on 16-bit machine: */
		assert(strm.avail_in == len);
		strm.next_in = (z_const Bytef*)ptr;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			if(result.append(out, have))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
