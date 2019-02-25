#ifndef __UTIL_PATH_H__
#define __UTIL_PATH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sys/types.h>

int path_isabs(const char *path);
char *path_suffix(const char *path);
char *path_basename(const char *path);

// alloc memory
char *path_dirname(const char *path);

int path_is_r_ok(const char *path);
int path_is_w_ok(const char *path);
int path_is_x_ok(const char *path);
int path_exists(const char *path);
int path_isfile(const char *path);
int path_islink(const char *path);
int path_isdir(const char *path);
int path_issamefile(const char *path1, const char *path2);
off_t path_getsize(const char *path);
time_t path_getatime(const char *path);
time_t path_getmtime(const char *path);
time_t path_getctime(const char *path);
int path_getamctime(const char *path, time_t *st_atm, time_t *st_mtm, time_t *st_ctm);

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_PATH_H__ */
