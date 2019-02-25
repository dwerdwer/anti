#ifndef __UTIL_STRING_H__
#define __UTIL_STRING_H__

// #ifdef __cplusplus
// extern "C" {
// #endif

char *str_strip(char *str);
char *get_value_by_key(const char *haystack, const char *needle, char sep, int ignore_case);

void str_split_free(char **ptr, int num);
char **str_split(const char *str, const char sep, int *num_toks);
#ifdef STRING_DEBUG
void str_split_debug(char **ptr, int num);
#endif /* STRING_DEBUG */


#if 0
void str_split_free(char*** p_buff, int num_toks);
char** str_split(const char* str, const char* sep_chars, const int max_toks,
    int* num_toks, const char meta_char);
#endif


// #ifdef __cplusplus
// }
// #endif

#endif /*__UTIL_STRING_H__*/
