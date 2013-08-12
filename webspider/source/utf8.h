#ifndef _utf8_h_
#define _utf8_h_

#ifdef  __cplusplus
extern "C" {
#endif

int utf8_to_gb18030(const char* src, char* tgt, int tgtBytes);

int gb18030_to_utf8(const char* src, char* tgt, int tgtBytes);

int to_utf8(const char* text, const char* encoding, char* utf8, int utf8Len);

#ifdef  __cplusplus
}
#endif

#endif /* !_utf8_h_ */
