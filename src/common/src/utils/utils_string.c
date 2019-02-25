
#include "utils_string.h"

#include <string.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////
// Private Implementation
////////////////////////////////////////////////////////////////

static uint32_t copy_chars(const char *p_src_string, uint32_t from_index, uint32_t to_index,
        char *p_dest_buffer, uint32_t dest_char_count)
{
    uint32_t result = 0;
    if (to_index > from_index)
    {
        uint32_t expected_length = to_index - from_index;
        uint32_t length = 
            (expected_length < dest_char_count) ? expected_length : dest_char_count;
        strncpy(p_dest_buffer, p_src_string + from_index, length);
        result = length; 
    }
    return result;
}

static uint32_t copy_wchars(const wchar_t *p_src_string, uint32_t from_index, uint32_t to_index,
        wchar_t *p_dest_buffer, uint32_t dest_char_count)
{
    uint32_t result = 0;
    if (to_index > from_index)
    {
        uint32_t expected_length = to_index - from_index;
        uint32_t length = 
            (expected_length < dest_char_count) ? expected_length : dest_char_count;
        wcsncpy(p_dest_buffer, p_src_string + from_index, length);
        result = length;
    }
    return result;
}

////////////////////////////////////////////////////////////////
// Public Interfaces
////////////////////////////////////////////////////////////////

void collapse_char_in_string(const char *p_src_string, char target_char,
        char *p_dest_buffer, uint32_t dest_char_count)
{
    uint32_t count = 0;
    uint32_t from_index = 0;
    uint32_t current_index = 0;
    int32_t is_collapsing = 0;

    while(0 != p_src_string[current_index] && current_index < dest_char_count)
    {
        if (0 == is_collapsing)
        {
            if (target_char == p_src_string[current_index])
            {
                count += copy_chars(p_src_string, from_index, current_index, 
                        p_dest_buffer + count, dest_char_count - count);
                from_index = current_index;
                is_collapsing = 1;
            }
        }
        else
        {
            if (target_char != p_src_string[current_index])
            {
                is_collapsing = 0;
            }
            else
            {
                from_index = current_index; 
            }
        }
        ++current_index; 
    }
    // Do not forget the last piece of string.
    copy_chars(p_src_string, from_index, current_index, 
            p_dest_buffer + count, dest_char_count - count);
}

void collapse_wchar_in_string(const wchar_t* p_src_string, wchar_t target_char,
        wchar_t *p_dest_buffer, uint32_t dest_char_count)
{
    uint32_t count = 0;
    uint32_t from_index = 0;
    uint32_t current_index = 0;
    int32_t is_collapsing = 0;

    while(L'\0' != p_src_string[current_index] && current_index < dest_char_count)
    {
        if (0 == is_collapsing)
        {
            if (target_char == p_src_string[current_index])
            {
                count += copy_wchars(p_src_string, from_index, current_index, 
                        p_dest_buffer + count, dest_char_count - count);
                from_index = current_index;
                is_collapsing = 1;
            }
        }
        else
        {
            if (target_char != p_src_string[current_index])
            {
                is_collapsing = 0;
            }
            else
            {
                from_index = current_index; 
            }
        }
        ++current_index; 
    }
    // Do not forget the last piece of string.
    copy_wchars(p_src_string, from_index, current_index, 
            p_dest_buffer + count, dest_char_count - count);
}

char *copy_string(const char *p_string)
{
    // Do not forget the terminating null.
    uint64_t string_length = strlen(p_string) + 1;
    char *p_result = (char*)malloc(string_length);
    if (NULL != p_result)
    {
        strncpy(p_result, p_string, string_length);
    }
    return p_result;
}


