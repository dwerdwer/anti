#ifndef __UTIL_STRING_H__
#define __UTIL_STRING_H__
#include <inttypes.h>

char** str_split(const char* str, const char* sep_chars, const int max_toks,
    int* num_toks, const char meta_char);

void str_split_free(char*** p_buff, int num_toks);


// s1+s2 -> s3
// must free(s3)
char *str_append(const char *str_front, const char *str_behind);
char *str_n_append(const char *str_front, size_t len_front, const char *str_behind, size_t len_behind);

#endif /* __UTIL_STRING_H__ */
