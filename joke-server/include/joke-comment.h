#ifndef _joke_comment_h_
#define _joke_comment_h_

#include "time64.h"
#include <string>

int jokecomment_init();
int jokecomment_save();

int jokecomment_query(unsigned int id, time64_t& datetime, std::string& comment);
int jokecomment_insert(unsigned int id, time64_t datetime, const std::string& comment);

#endif /* !_joke_comment_h_ */
