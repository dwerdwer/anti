#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* str_split_add_tok(const char* str, const int len, const char* sep_chars, const char meta_char)
{
    size_t i, j, k;
    char* tok;
    int tok_len = 0;
    int got_meta = 0;

    /* Get the length of the returned tok
     * Could have a maximum token length and use a fixed sized array and
     * fill it in as we go but don't want to put on that constraint */
    for (i = 0; (int)i < len; i++)
    {
        if (!got_meta)
        {
            if (str[i] == meta_char)
            {
                got_meta = 1;
                continue;
            }
        }
        else
        {
            /* See if the current character is a separator */
            for (j = 0; j < strlen(sep_chars); j++)
            {
                if (str[i] == sep_chars[j])
                    break;
            }

            /* It's a non-separator character, so include
             * the meta character in the return tok */
            if (j == strlen(sep_chars))
                tok_len++;

            got_meta = 0;
        }

        tok_len++;
    }

    /* Allocate it and fill it in */
    tok = (char*)calloc(1, tok_len + 1);
    for (i = 0, k = 0; (int)i < len; i++)
    {
        if (!got_meta)
        {
            if (str[i] == meta_char)
            {
                got_meta = 1;
                continue;
            }
        }
        else
        {
            /* See if the current character is a separator */
            for (j = 0; j < strlen(sep_chars); j++)
            {
                if (str[i] == sep_chars[j])
                    break;
            }

            /* It's a non-separator character, so include
             * the meta character in the return tok */
            if (j == strlen(sep_chars))
                tok[k++] = meta_char;

            got_meta = 0;
        }

        tok[k++] = str[i];
    }

    return tok;
}

static int str_snort_strncpy(char *dst, const char *src, size_t dst_size)
{
    char *ret = NULL;

    if (dst == NULL || src == NULL || dst_size <= 0)
        return -1;

    dst[dst_size - 1] = '\0';
    ret = strncpy(dst, src, dst_size);

    if (ret == NULL)
        return -1;

    if (dst[dst_size - 1] != '\0') {
        dst[dst_size - 1] = '\0';
        return 1;
    }

    return 0;
}

static char *str_snort_strndup(const char *src, size_t dst_size)
{
    char *ret = (char *)calloc(1, dst_size + 1);
    int ret_val;

    ret_val = str_snort_strncpy(ret, src, dst_size + 1);

    if(ret_val == -1) {
        free(ret);
        return NULL;
    }

    return ret;
}

#define TOKS_BUF_SIZE   100
/****************************************************************
 *
 * Free the buffer allocated by str_split().
 *
 * char** toks = NULL;
 * int num_toks = 0;
 * toks = (str, " ", 2, &num_toks, 0);
 * str_split_free(&toks, num_toks);
 *
 * At this point, toks is again NULL.
 *
 ****************************************************************/
void str_split_free(char*** p_buff, int num_toks)
{
    int i;
    char** buf;  /* array of string pointers */

    if ( p_buff==NULL || *p_buff==NULL )
    {
        return;
    }

    buf = *p_buff;

    for ( i=0; i<num_toks; i++ )
    {
        if ( buf[i] != NULL )
        {
            free(buf[i]);
            buf[i] = NULL;
        }
    }

    free(buf);
    *p_buff = NULL;
}

/****************************************************************
 *
 * Function: str_split()
 *
 * Purpose: Splits a string into tokens non-destructively.
 *
 * Parameters:
 *  char *
 *      The string to be split
 *  char *
 *      A string of token seperaters
 *  int
 *      The maximum number of tokens to be returned. A value
 *      of 0 means to get them all.
 *  int *
 *      Place to store the number of tokens returned
 *  char
 *      The "escape metacharacter", treat the character after
 *      this character as a literal and "escape" a seperator.
 *
 *  Note if max_toks is reached, the last tok in the returned
 *  token array will possibly have separator characters in it.
 *
 *  Returns:
 *      2D char array with one token per "row" of the returned
 *      array.
 *
 ****************************************************************/

char** str_split(const char* str, const char* sep_chars, const int max_toks,
    int* num_toks, const char meta_char)
{
    size_t cur_tok = 0;  /* current token index into array of strings */
    size_t tok_start;    /* index to start of token */
    size_t i, j;
    int escaped = 0;
    /* It's rare we'll need more than this even if max_toks is set really
     * high.  Store toks here until finished, then allocate.  If more than
     * this is necessary, then allocate max toks */
    char* toks_buf[TOKS_BUF_SIZE];
    size_t toks_buf_size = TOKS_BUF_SIZE;
    int toks_buf_size_increment = 10;
    char** toks_alloc = NULL;   /* Used if the static buf isn't enough */
    char** toks = toks_buf;     /* Pointer to one of the two above */
    char** retstr;
    const char* whitespace = " \t";

    if (num_toks == NULL)
        return NULL;

    *num_toks = 0;

    if ((str == NULL) || (strlen(str) == 0) ||
        ((sep_chars != NULL) && (strlen(sep_chars) == 0)))
    {
        return NULL;
    }

    if (sep_chars == NULL)
        sep_chars = whitespace;

    /* Meta char cannot also be a separator char */
    for (i = 0; i < strlen(sep_chars); i++)
    {
        if (sep_chars[i] == meta_char)
            return NULL;
    }

    /* Move past initial separator characters and whitespace */
    for (i = 0; i < strlen(str); i++)
    {
        for (j = 0; j < strlen(sep_chars); j++)
        {
            if ((str[i] == sep_chars[j]) ||
                isspace((int)str[i]))
            {
                break;
            }
        }

        /* Not a separator character or whitespace */
        if (j == strlen(sep_chars))
            break;
    }

    if (i == strlen(str))
    {
        /* Nothing but separator characters or whitespace in string */
        return NULL;
    }

    /* User only wanted one tok so return the rest of the string in
     * one tok */
    if ((cur_tok + 1) == (size_t)max_toks)
    {
        retstr = (char**)calloc(1, sizeof(char*));
        retstr[cur_tok] = str_snort_strndup(&str[i], strlen(str) - i);
        if (retstr[cur_tok] == NULL)
        {
            str_split_free(&retstr, cur_tok + 1);
            return NULL;
        }

        *num_toks = cur_tok + 1;
        return retstr;
    }

    /* Mark the beginning of the next tok */
    tok_start = i;
    for (; i < strlen(str); i++)
    {
        if (!escaped)
        {
            /* Got an escape character.  Don't include it now, but
             * must be a character after it. */
            if (str[i] == meta_char)
            {
                escaped = 1;
                continue;
            }

            /* See if the current character is a separator */
            for (j = 0; j < strlen(sep_chars); j++)
            {
                if (str[i] == sep_chars[j])
                    break;
            }

            /* It's a normal character */
            if (j == strlen(sep_chars))
                continue;

            /* Current character matched a separator character.  Trim off
             * whitespace previous to the separator.  If we get here, there
             * is at least one savable character */
            for (j = i; j > tok_start; j--)
            {
                if (!isspace((int)str[j - 1]))
                    break;
            }

            /* Allocate a buffer.  The length will not have included the
             * meta char of escaped separators */
            toks[cur_tok] = str_split_add_tok(&str[tok_start], j - tok_start, sep_chars, meta_char);

            /* Increment current token index */
            cur_tok++;

            /* Move past any more separator characters or whitespace */
            for (; i < strlen(str); i++)
            {
                for (j = 0; j < strlen(sep_chars); j++)
                {
                    if ((str[i] == sep_chars[j]) ||
                        isspace((int)str[i]))
                    {
                        break;
                    }
                }

                /* Not a separator character or whitespace */
                if (j == strlen(sep_chars))
                    break;
            }

            /* Nothing but separator characters or whitespace left in the string */
            if (i == strlen(str))
            {
                *num_toks = cur_tok;

                if (toks != toks_alloc)
                {
                    retstr = (char**)calloc(1, sizeof(char*) * cur_tok);
                    memcpy(retstr, toks, (sizeof(char*) * cur_tok));
                }
                else
                {
                    retstr = toks;
                }

                return retstr;
            }

            /* Reached the size of our current string buffer and need to
             * allocate something bigger.  Only get here once if max toks
             * set to something other than 0 because we'll just allocate
             * max toks in that case. */
            if (cur_tok == toks_buf_size)
            {
                char** tmp;

                if (toks_alloc != NULL)
                    tmp = toks_alloc;
                else
                    tmp = toks_buf;

                if (max_toks != 0)
                    toks_buf_size = max_toks;
                else
                    toks_buf_size = cur_tok + toks_buf_size_increment;

                toks_alloc = (char**)calloc(1, sizeof(char*) * toks_buf_size);
                memcpy(toks_alloc, tmp, (sizeof(char*) * cur_tok));
                toks = toks_alloc;

                if (tmp != toks_buf)
                    free(tmp);
            }

            if ((max_toks != 0) && ((cur_tok + 1) == (size_t)max_toks))
            {
                /* Return rest of string as last tok */
                *num_toks = cur_tok + 1;

                /* Already got a ret string */
                if (toks != toks_alloc)
                {
                    retstr = (char**)calloc(1, sizeof(char*) * (cur_tok + 1));
                    memcpy(retstr, toks, (sizeof(char*) * (cur_tok + 1)));
                }
                else
                {
                    retstr = toks;
                }

                /* Trim whitespace at end of last tok */
                for (j = strlen(str); j > tok_start; j--)
                {
                    if (!isspace((int)str[j - 1]))
                        break;
                }

                retstr[cur_tok] = str_snort_strndup(&str[i], j - i);
                if (retstr[cur_tok] == NULL)
                {
                    str_split_free(&retstr, cur_tok + 1);
                    return NULL;
                }

                return retstr;
            }

            tok_start = i;
        }
        else
        {
            /* This character is escaped with the meta char */
            escaped = 0;
        }
    }

    /* Last character was an escape character */
    if (escaped)
    {
        for (i = 0; i < cur_tok; i++)
            free(toks[i]);

        if (toks == toks_alloc)
            free(toks_alloc);

        return NULL;
    }

    /* Trim whitespace at end of last tok */
    for (j = i; j > tok_start; j--)
    {
        if (!isspace((int)str[j - 1]))
            break;
    }

    /* Last character was not a separator character so we've got
     * one more tok.  Unescape escaped sepatator charactors */
    if (toks != toks_alloc)
    {
        retstr = (char**)calloc(1, sizeof(char*) * (cur_tok + 1));
        memcpy(retstr, toks, (sizeof(char*) * (cur_tok + 1)));
    }
    else
    {
        retstr = toks;
    }

    retstr[cur_tok] = str_split_add_tok(&str[tok_start], j - tok_start, sep_chars, meta_char);

    /* Just add one to cur_tok index instead of incrementing
     * since we're done */
    *num_toks = cur_tok + 1;
    return retstr;
}

#include <assert.h>
char *str_append(const char *str_front, const char *str_behind)
{
    size_t len_front = strlen(str_front);
    size_t len_behind = strlen(str_behind);
    char *ret = (char *)malloc(len_front + len_behind + 1);
    assert(ret);
    strncpy(ret          , str_front,  len_front);
    strncpy(ret+len_front, str_behind, len_behind);
    ret[len_front + len_behind] = '\0';
    return ret;
}

char *str_n_append(const char *str_front, size_t len_front, const char *str_behind, size_t len_behind)
{
    char *ret = (char *)malloc(len_front + len_behind + 1);
    assert(ret);
    strncpy(ret          , str_front,  len_front);
    strncpy(ret+len_front, str_behind, len_behind);
    ret[len_front + len_behind] = '\0';
    return ret;
}
