
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <string.h>


// ret: -1: error, see ERRNO
//      0 : success
int makedir(std::string dir, mode_t mode)
{
    struct stat st;

    if (dir.size() == 0)
    {
        // printf("%s [%s] failed\n", __func__, dir.c_str());
        return -1;
    }

    if (dir[dir.size() - 1] == '/')
    {
        dir.erase(dir.size() - 1);
    }

    if (dir.size() == 0)
    {
        // printf("%s [%s] size is 0\n", __func__, dir.c_str());
        return 0;
    }

    if (stat(dir.data(), &st))
    {
        return mkdir(dir.data(), mode);
    }

    if (!S_ISDIR(st.st_mode))
    {
        if (unlink(dir.data()))
        {
            // printf("%s [%s] unlink failed\n", __func__, dir.c_str());
            return -1;
        }
        return mkdir(dir.data(), mode);
    }
    return 0;
}

int makedir_p(std::string dir, mode_t mode)
{
    int ret = -1;
    ret = makedir(dir.data(), mode);
    if (ret == 0)
    {
        // printf("%s [%s] success\n", __func__, dir.c_str());
        return 0;
    }

    // parse parent string
    if (dir[dir.size() - 1] == '/')
    {
        dir.erase(dir.size() - 1);
    }

    size_t off = dir.find_last_of('/');
    if (off != std::string::npos)
    {
        std::string parentdir = dir.substr(0, off + 1);
        if(parentdir.size() == 0 || parentdir.size() == 1)
        {
            // printf("%s [%s] ok\n", __func__, dir.c_str());
            return 0;
        }
        ret = makedir_p(parentdir, mode);
        if(ret == -1)
        {
            // printf("%s [%s] failed, errno:%d\n", __func__, parentdir.c_str(), errno);
            return ret;
        }
        else
        {
            // printf("%s [%s] success\n", __func__, parentdir.c_str());
            return makedir(dir.data(), mode);
        }
    }
    // printf("%s [%s] failed, errno:%d\n", __func__, dir.c_str(), errno);
    return -1;
}

bool str_endswith(const char *str, size_t str_len, const char *sub, size_t sub_len)
{
    if(str_len >= sub_len && sub_len) {
        return (strncmp(str+str_len-sub_len, sub, sub_len) == 0);
    }
    return false;
}

bool str_endswith(const char *str, const char *sub)
{
    return str_endswith(str, strlen(str), sub, strlen(sub));
}

bool str_endswith(std::string &str, std::string &sub)
{
    return str_endswith(str.data(), str.size(), sub.data(), sub.size());
}

bool str_endswith(std::string &str, const char *sub)
{
    return str_endswith(str.data(), str.size(), sub, strlen(sub));
}

std::string str_replace(std::string str, const char *src, const char *dst)
{
    size_t pos = 0;
    size_t src_len = strlen(src);
    while(true) {
        pos = str.find(src, pos);
        if(pos == std::string::npos)
            break;
        str.replace(pos, src_len, dst);
    }
    return str;
}

std::string str_replace(const char *str, const char *src, const char *dst)
{
    return str_replace(std::string (str), src, dst);
}

std::string path_format(std::string path)
{
    if(path.size() == 0)
        return path;

    if(str_endswith(path, "/.")
        || str_endswith(path, "/.."))
    {
        path += "/";
    }

    path = str_replace(path, "//", "/");
    path = str_replace(path, "/./", "/");
    size_t pos = 0;
    static std::string recurse_flag = "/../";
    while(true) {
        pos = path.find(recurse_flag, pos);
        if(pos == std::string::npos)
            break;
        size_t prev_pos = path.find_last_of("/", pos-1);
        // this an root path
        if(prev_pos == std::string::npos)
        {
            return "/";
        }
        path.erase(prev_pos+1, (pos+recurse_flag.size()) -(prev_pos+1));
    }
    if(str_endswith(path, "/")) {
        path.erase(path.size()-1);
    }
    if(str_endswith(path, "/.")) {
        path.erase(path.size()-2);
    }
    if(str_endswith(path, "/..")) {
        pos = path.find_last_of("/", path.size()-4);
        if(pos != std::string::npos) {
            path.erase(pos);
        }
    }
    return path;
}


/* exactly tok the index-th string */
std::string tok(const std::string str, const std::string sep, size_t index)
{
    size_t ppos = 0, cpos = 0;
    do
    {
        if(cpos)
        {
            ppos = cpos + sep.size();
        }
        cpos = str.find(sep, ppos);

        if(cpos == std::string::npos)
        {
            cpos = str.size();
            break;
        }

    } while(index--);

    return str.substr(ppos, cpos - ppos);
}

std::string tok_key(const std::string str, const std::string sep)
{
    return tok(str, sep, 0);
}
std::string tok_value(const std::string str, const std::string sep)
{
    /* Maybe value contains one or more sep strings */
    return str.substr(tok_key(str, sep).size() + sep.size());
}
