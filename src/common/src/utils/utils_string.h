#ifndef UTILS_STRING_HHH
#define UTILS_STRING_HHH

#include <stdint.h>
#include <wchar.h>

void collapse_char_in_string(const char* p_src_string, char target_char,
        char *p_dest_buffer, uint32_t dest_char_count);

void collapse_wchar_in_string(const wchar_t* p_src_string, wchar_t target_char,
        wchar_t *p_dest_buffer, uint32_t dest_char_count);

char *copy_string(const char *p_string);

#endif

