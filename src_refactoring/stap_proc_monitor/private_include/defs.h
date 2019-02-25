#ifndef __DEFINES_H__
#define __DEFINES_H__


#define MAX_LINE_SIZE                       1024
#define MAX_ADDR_STR_SIZE                   36
#define MAX_LEN_FILE_NAME                   256
#define MAX_LEN_ABSOLUTE_PATH               1024
#define MAX_LEN_ABSOLUTE_PATH_X_FILE_NAME   (MAX_LEN_ABSOLUTE_PATH + MAX_LEN_FILE_NAME)
#define MAX_LEN_CMD_LINE                    256

#define xcalloc     calloc
#define xmalloc     malloc
#define xrealloc    realloc
#define xfree       free

#define xstrdup     strdup

#define likely(x)       (x)
#define unlikely(x)     (x)
#define expected(x,y)   (x)

#ifndef MIN
#define MIN(a, b) ((a)>(b)?(b):(a))
#endif
#ifndef MAX
#define MAX(a, b) ((a)<(b)?(b):(a))
#endif

#ifdef  __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif


#define INFO_PRINT(...)\
        printf("[%s] ", __func__);\
        printf(__VA_ARGS__);

#define WARN_PRINT(...)\
        fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);



#endif // __DEFINES_H__
