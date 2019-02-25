
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fstream>

#include "version_manager.h"
#include "upgrade_logger.h"

#define LEFTSTRIP 0
#define RIGHTSTRIP 1
#define BOTHSTRIP 2


/* exactly tok the index-th string */
static std::string tok(const std::string str, const std::string sep, size_t index)
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

static std::string tok_key(const std::string str, const std::string sep)
{
    return tok(str, sep, 0);
}
static std::string tok_value(const std::string str, const std::string sep)
{
    /* Maybe value contains one or more sep strings */
    return str.substr(tok_key(str, sep).size() + sep.size());
}

static std::string do_strip(const std::string &str, int striptype, const std::string&chars)
{
    std::string::size_type str_len = str.size();
    std::string::size_type chars_len = chars.size();
    std::string::size_type i, j;

    //默认情况下，去除空白符
    if (0 == chars_len)
    {
        i = 0;
        //去掉左边空白字符
        if (striptype != RIGHTSTRIP)
        {
            while (i < str_len&&::isspace(str[i]))
            {
                i++;
            }
        }
        j = str_len;
        //去掉右边空白字符
        if (striptype != LEFTSTRIP)
        {
            j--;
            while (j >= i&&::isspace(str[j]))
            {
                j--;
            }
            j++;
        }
        return str.substr(i, j - i);
    }
    else
    {
        //把删除序列转为c字符串
        const char*sep = chars.c_str();
        i = 0;
        if (striptype != RIGHTSTRIP)
        {
            //memchr函数：从sep指向的内存区域的前charslen个字节查找str[i]
            while (i < str_len&&memchr(sep, str[i], chars_len))
            {
                i++;
            }
        }
        j = str_len;
        if (striptype != LEFTSTRIP)
        {
            j--;
            while (j >= i&&memchr(sep, str[j], chars_len))
            {
                j--;
            }
            j++;
        }
        //如果无需要删除的字符
        if (0 == i&& j == str_len)
        {
            return str;
        }
        else
        {
            return str.substr(i, j - i);
        }
    }

}

static std::string strip( const std::string & str, const std::string & chars="" )
{
    return do_strip( str, BOTHSTRIP, chars );
}
static std::string lstrip( const std::string & str, const std::string & chars="" )
{
    return do_strip( str, LEFTSTRIP, chars );
}
static std::string rstrip( const std::string & str, const std::string & chars="" )
{
    return do_strip( str, RIGHTSTRIP, chars );
}

// static std::string readfile(const char *filename)
// {
//     std::string cont;
//     if (FILE *fp = fopen(filename, "r"))
//     {
//         char buf[1024];
//         while (size_t len = fread(buf, 1, sizeof(buf), fp))
//             cont.insert(cont.end(), buf, buf + len);
//         fclose(fp);
//     }
//     return cont;
// }

// static size_t writefile(const char *filename, const char *cont, size_t sz)
// {
//     if (FILE *fp = fopen(filename, "w"))
//     {
//         size_t offset = 0;
//         while (offset += fwrite(cont + offset, 1, sz - offset, fp))
//         {
//             if(offset == sz)
//             {
//                 break;
//             }
//         }
//         fclose(fp);
//         return offset;
//     }
//     return 0;
// }

// VersionManager::VersionManager()
// {
// }

VersionManager::~VersionManager()
{

}

bool VersionManager::init(std::string file)
{
    std::ifstream ifs(file);
    while(ifs)
    {
        std::string line;
        std::getline(ifs, line);
        if(tok_key(line, SEPERATOR).compare(this->get_key()) == 0)
        {
            this->_ver = strip(tok_value(line, SEPERATOR));
            g_logger.log("VersionManager %s curr version -> [%s]", this->get_key().c_str(), this->_ver.c_str());
            return true;
        }
    }
    return false;
}

bool VersionManager::check_upgrade(std::string file)
{
    std::ifstream ifs(file);
    while(ifs)
    {
        std::string line;
        std::getline(ifs, line);
        if(tok_key(line, SEPERATOR).compare(this->get_key()) == 0)
        {
            std::string dst = strip(tok_value(line, SEPERATOR));
            g_logger.log("VersionManager %s dest version -> [%s]", this->get_key().c_str(), this->_ver.c_str());
            return this->compare(dst);
        }
    }
    return false;
}

std::string VersionManager::curr_version()
{
    return this->_ver;
}

// class DaemonVersionManager : public VersionManager
// {
// public:
//     virtual bool compare(const char *, size_t);
//     virtual bool is_version_legal(const char *, size_t);
// };


// class VirusLibVersionManager : public VersionManager
// {
// public:
//     virtual bool compare(const char *, size_t);
//     virtual bool is_version_legal(const char *, size_t);
// };

std::string DaemonVersionManager::get_key()
{
    return DAEMON_KEY;
}

bool DaemonVersionManager::compare(std::string destversion)
{
    std::string currversion = this->curr_version();
    bool ret = false;

    /* version format such as : 123.456.789 */
#define VERSION_SEP "."
#define VERSION_TOKS 3

    for(size_t i = 0; i < VERSION_TOKS; i++)
    {
        size_t curr = std::stoul(tok(currversion, VERSION_SEP, i));
        size_t dest = std::stoul(tok(destversion, VERSION_SEP, i));

        if(curr < dest)
        {
            ret = true;
            break;
        }
        else if(curr > dest)
        {
            ret = false;
            break;
        }

        /* continue */
    }
    g_logger.log("%s %s [%s] ==> [%s], ret:%d", __func__, this->get_key().c_str(), currversion.c_str(), destversion.c_str(), ret);
    return ret;
}

bool DaemonVersionManager::is_version_legal(std::string str)
{
    /* str is [0-9] or '.'
     * str startswith [0-9]
     */
    if(!std::isdigit(str[0]))
    {
        g_logger.log("%s %s [%s] format error", __func__, this->get_key().c_str(), str.c_str());
        return false;
    }
    for(size_t i = 0; i < str.size(); i++)
    {
        if((!std::isdigit(str[i])) && (str[i] != '.'))
        {
            g_logger.log("%s %s [%s] format error", __func__, this->get_key().c_str(), str.c_str());
            return false;
        }
    }
    return true;
}

std::string VirusLibVersionManager::get_key()
{
    return VIRUSLIB_KEY;
}

bool VirusLibVersionManager::compare(std::string destversion)
{
    time_t cur = std::stoul(this->curr_version());
    time_t dst = std::stoul(destversion);
    return dst > cur;
}

bool VirusLibVersionManager::is_version_legal(std::string str)
{
    for(size_t i = 0; i < str.size(); i++)
    {
        if(!std::isdigit(str[i]))
        {
            g_logger.log("%s %s [%s] format error", __func__, this->get_key().c_str(), str.c_str());
            return false;
        }
    }
    return true;
}
