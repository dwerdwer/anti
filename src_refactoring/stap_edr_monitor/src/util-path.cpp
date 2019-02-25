#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// #include <stdbool.h>

#include "util-path.h"

int path_isabs(const char *path)
{
    return (path ? ('/' == path[0] ? 1: 0) : 0);
}

char *path_suffix(const char *path)
{
    char *suffix = (char *)path;
    if(path)
    {
        suffix = strrchr((char *)path, '.');
        if(suffix)
            suffix++;
    }
    return suffix;
}

char *path_basename(const char *path)
{
    char *basename = (char *)path;
    if(path)
    {
        basename = strrchr((char *)path, '/');
        if(basename)
            basename++;
    }
    return basename;
}

char *path_dirname(const char *path)
{
    char *basename = path_basename(path);
    return strndup(path, basename-path);
}

//char *path_join(const char *path1, const char *path2)
//{
//
//}

int path_is_r_ok(const char *path)
{
    return 0 == access(path, R_OK);
}

int path_is_w_ok(const char *path)
{
    return 0 == access(path, W_OK);
}

int path_is_x_ok(const char *path)
{
    return 0 == access(path, X_OK);
}

int path_exists(const char *path)
{
    return 0 == access(path, F_OK);
}

int path_isfile(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return S_ISREG(st.st_mode);
    }
    return 0;
}

int path_islink(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return S_ISLNK(st.st_mode);
    }
    return 0;
}

int path_isdir(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return S_ISDIR(st.st_mode);
    }
    return 0;
}

int path_issamefile(const char *path1, const char *path2)
{
    struct stat st1, st2;
    if(stat(path1, &st1) || stat(path2, &st2))
        return 0;
    return st1.st_ino == st2.st_ino;
}

off_t path_getsize(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return st.st_size;
    }
    return -1;
}

time_t path_getatime(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return (time_t)st.st_atime;
    }
    return 0;
}

time_t path_getmtime(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return (time_t)st.st_mtime;
    }
    return 0;
}

time_t path_getctime(const char *path)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        return (time_t)st.st_ctime;
    }
    return 0;
}

int path_getamctime(const char *path, time_t *st_atm, time_t *st_mtm, time_t *st_ctm)
{
    struct stat st;
    if(0 == stat(path, &st))
    {
        if(st_atm)
            *st_atm = (time_t)st.st_atime;
        if(st_mtm)
            *st_mtm = (time_t)st.st_mtime;
        if(st_ctm)
            *st_ctm = (time_t)st.st_ctime;
        return 0;
    }
    return -1;
}

#ifdef PATH_MAIN

#define TEST_PATH1  "/bin/sh"
#define TEST_PATH2  "./a.out"
#define TEST_PATH3  "../"
#define TEST_PATH4  "../usr/bin/sh"
#define TEST_PATH5  "/usr/bin/python"
#define TEST_PATH6  "/usr/bin/python2"

int main(void)
{
    int res = 0;

    printf("path_isabs(%s) --> %d\n", TEST_PATH1, path_isabs(TEST_PATH1));
    printf("path_isabs(%s) --> %d\n", TEST_PATH2, path_isabs(TEST_PATH2));
    printf("path_isabs(%s) --> %d\n", TEST_PATH4, path_isabs(TEST_PATH3));

    printf("path_exists(%s) --> %d\n", TEST_PATH1, path_exists(TEST_PATH1));
    printf("path_exists(%s) --> %d\n", TEST_PATH2, path_exists(TEST_PATH2));
    printf("path_exists(%s) --> %d\n", TEST_PATH3, path_exists(TEST_PATH3));

    time_t atime, mtime, ctime;
    {
        printf("%s\n", TEST_PATH1);
        printf("\tpath_isabs(%s)   --> %d\n", TEST_PATH1, path_isabs(TEST_PATH1));
        printf("\tpath_exists(%s)     --> %d\n", TEST_PATH1, path_exists(TEST_PATH1));
        if(path_exists(TEST_PATH1))
            printf("\tpath_getsize(%s)     --> %lld\n", TEST_PATH1, path_getsize(TEST_PATH1));
        if(0 == path_getamctime(TEST_PATH1, &atime, &mtime, &ctime))
            printf("\tpath_getamctime(%s) --> atime:%lu, mtime:%lu, ctime:%lu\n", TEST_PATH1, atime, mtime, ctime);
    }
    {
        printf("%s\n", TEST_PATH2);
        printf("\tpath_isabs(%s)   --> %d\n", TEST_PATH2, path_isabs(TEST_PATH2));
        printf("\tpath_exists(%s)     --> %d\n", TEST_PATH2, path_exists(TEST_PATH2));
        if(path_exists(TEST_PATH2))
            printf("\tpath_getsize(%s)     --> %lld\n", TEST_PATH2, path_getsize(TEST_PATH2));
        if(0 == path_getamctime(TEST_PATH2, &atime, &mtime, &ctime))
            printf("\tpath_getamctime(%s) --> atime:%lu, mtime:%lu, ctime:%lu\n", TEST_PATH2, atime, mtime, ctime);
    }
    {
        printf("%s\n", TEST_PATH3);
        printf("\tpath_isabs(%s)   --> %d\n", TEST_PATH3, path_isabs(TEST_PATH3));
        printf("\tpath_exists(%s)     --> %d\n", TEST_PATH3, path_exists(TEST_PATH3));
        if(path_exists(TEST_PATH3))
            printf("\tpath_getsize(%s)     --> %lld\n", TEST_PATH3, path_getsize(TEST_PATH3));
        if(0 == path_getamctime(TEST_PATH3, &atime, &mtime, &ctime))
            printf("\tpath_getamctime(%s) --> atime:%lu, mtime:%lu, ctime:%lu\n", TEST_PATH3, atime, mtime, ctime);
    }
    {
        printf("%s\n", TEST_PATH4);
        printf("\tpath_isabs(%s)   --> %d\n", TEST_PATH4, path_isabs(TEST_PATH4));
        printf("\tpath_exists(%s)     --> %d\n", TEST_PATH4, path_exists(TEST_PATH4));
        if(path_exists(TEST_PATH4))
            printf("\tpath_getsize(%s)     --> %lld\n", TEST_PATH4, path_getsize(TEST_PATH4));
        if(0 == path_getamctime(TEST_PATH4, &atime, &mtime, &ctime))
            printf("\tpath_getamctime(%s) --> atime:%lu, mtime:%lu, ctime:%lu\n", TEST_PATH4, atime, mtime, ctime);
    }
    printf("path_issamefile(%s, %s) --> %d\n", TEST_PATH5, TEST_PATH6, path_issamefile(TEST_PATH5, TEST_PATH6));
    //    printf("path_getsize(%s) --> %ld\n", TEST_PATH4, path_getsize(TEST_PATH4));
    return 0;
}
#endif


