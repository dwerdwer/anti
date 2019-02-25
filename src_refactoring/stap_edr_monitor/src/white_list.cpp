#include <stdio.h>
#include <time.h>

#include <string>
#include <map>
#include <vector>

#include <string.h>
#include "defs.h"
#include "white_list.h"
#include "util-path.h"

const char *default_config_file = "./FileWhite";
const char *default_config_proc = "./ProcessWhite";
const char *default_config_modu = "./ModWhite";
const char *default_config_host = "./IPWhite";

#ifdef WHITE_LIST_DEBUG
#define WHITE_LIST_DEBUG_PRINT(fmt, arg...) printf("whitelist " fmt, ##arg)

#define WHITE_LIST_DEBUG_MAP(map, detail)  debug_map(map, detail)
void debug_map(std::map<uint32_t, std::string> &map, const char *detail)
{
    printf("whitelist ---------------%s start---------------\n", detail);
    for(std::map<uint32_t, std::string>::iterator it=map.begin();
        it!=map.end(); it++)
    {
        printf("whitelist [%012u] -> [%s]\n", it->first, it->second.c_str());
    }
    printf("whitelist ---------------%s   end---------------\n", detail);
}
#else /* WHITE_LIST_DEBUG */
#define WHITE_LIST_DEBUG_PRINT(fmt, arg...) 
#define WHITE_LIST_DEBUG_MAP(map, detail)
#endif /* WHITE_LIST_DEBUG */

#define _rotl_KAZE(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define _rotl_KAZE64(x, n) (((x) << (n)) | ((x) >> (64-(n))))
static uint32_t fast_hash(const char *str, uint32_t wrdlen)
{
    const uint32_t PRIME = 709607;
    uint32_t hash32  = 2166136261UL;
    uint32_t hash32B = 2166136261UL;
    const char *p = str;
    uint32_t loops;
    uint32_t second_line;

    if( wrdlen >= 16 )
    {
        second_line = wrdlen-((wrdlen>>4)+1)*8; // ((wrdlen>>1)>>3)
        loops = (wrdlen>>4) + 1;
        for(;  loops; loops--, p += 8 )
        {
            // revision 1:
            //hash32 = (hash32 ^ (_rotl_KAZE(*(uint32_t *)(p+0),5) ^ *(uint32_t *)(p+4))) * PRIME;
            //hash32B = (hash32B ^ (_rotl_KAZE(*(uint32_t *)(p+0+second_line),5) ^ *(uint32_t *)(p+4+second_line))) * PRIME;
            // revision 2:
            hash32 = (hash32 ^ (_rotl_KAZE(*(uint32_t *)(p),5) ^ *(uint32_t *)(p+second_line))) * PRIME;
            hash32B = (hash32B ^ (_rotl_KAZE(*(uint32_t *)(p+4+second_line),5) ^ *(uint32_t *)(p+4))) * PRIME;
        }
    }
    else
    {
        if(wrdlen & 8 )
        {
            hash32 = (hash32 ^ *(uint32_t*)(p)) * PRIME;
            hash32B = (hash32B ^ *(uint32_t*)(p+4)) * PRIME;
            p += 4*sizeof(uint16_t);
        }

        if(wrdlen & 4 )
        {
            hash32 = (hash32 ^ *(uint16_t*)(p+0)) * PRIME;
            hash32B = (hash32B ^ *(uint16_t*)(p+2)) * PRIME;
            p += 2*sizeof(uint16_t);
        }
        if(wrdlen & 2 )
        {
            hash32 = (hash32 ^ *(uint16_t*)p) * PRIME;
            p += sizeof(uint16_t);
        }
        if(wrdlen & 1 )
            hash32 = (hash32 ^ *p) * PRIME;
    }

    hash32 = (hash32 ^ _rotl_KAZE(hash32B,5) ) * PRIME;
    return hash32 ^ (hash32 >> 16);
}

white_list *white_list::get_instance()
{
    static white_list white_list_instance;
    return &white_list_instance;
}

white_list::white_list() :
    config_file(default_config_file),
    config_proc(default_config_proc),
    config_modu(default_config_modu),
    config_host(default_config_host),
    file_last_load(0),
    proc_last_load(0),
    modu_last_load(0),
    host_last_load(0)
{
    printf("whitelist construct file:[%s], proc:[%s], modu:[%s], host:[%s]\n",
           config_file.c_str(), config_proc.c_str(), config_modu.c_str(), config_host.c_str());
}

void white_list::set_config_file(const char *file, const char *proc, 
        const char *modu, const char *host)
{
    if(file)
        config_file = file;
    if(proc)
        config_proc = proc;
    if(modu)
        config_modu = modu;
    if(host)
        config_host = host;
    printf("whitelist is: file:[%s] proc:[%s] module:[%s] ip:[%s]\n", 
        config_file.c_str(), config_proc.c_str(), config_modu.c_str(), config_host.c_str());
}

static int file_2_vector(const char *fname, std::vector<std::string> &content)
{
    FILE *file = fopen(fname, "r");
    if(!file)
    {
        WHITE_LIST_DEBUG_PRINT("open[%s] failed\n", fname);
        return -1;
    }
    char buffer[2048];
    const char *str_space = " \r\n\t";
    std::string line;
    while(NULL != fgets(buffer, sizeof(buffer), file))
    {
        line += buffer;
        if('\n' == buffer[strlen(buffer)-1])
        {
            size_t pos_start = line.find_first_not_of(str_space);
            size_t pos_end = line.find_last_not_of(str_space);
            if(std::string::npos != pos_start && std::string::npos != pos_start)
            {
                content.push_back(line.substr(pos_start, pos_end-pos_start+1));
            }
            line.clear();
        }
    }
    fclose(file);
    return 0;
}

static void vector_str_move_to_map_str(std::map<uint32_t, std::string> &map,
                std::vector<std::string> &vec)
{
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();)
    {
        map[fast_hash(it->c_str(), it->size())] = std::move(*it);
        it = vec.erase(it);
    }
}

void white_list::load_config()
{
    std::vector<std::string> content;
    time_t now = time(NULL);

    time_t file_last_change = MAX(path_getmtime(config_file.c_str()), path_getctime(config_file.c_str()));
    WHITE_LIST_DEBUG_PRINT("last_change:%lu, last_load:%lu, now:%lu\n", 
            file_last_change, file_last_load, now);
    if(0 == file_last_change || file_last_change > file_last_load)
    {
        this->file.clear();
        file_2_vector(config_file.c_str(), content);
        vector_str_move_to_map_str(this->file, content);
        file_last_load = now;
        content.clear();
        WHITE_LIST_DEBUG_MAP(this->file, "white_list_file");
    }

    time_t proc_last_change = MAX(path_getmtime(config_proc.c_str()), path_getctime(config_proc.c_str()));
    WHITE_LIST_DEBUG_PRINT("last_change:%lu, last_load:%lu, now:%lu\n", 
            proc_last_change, proc_last_load, now);
    if(0 == proc_last_change || proc_last_change > proc_last_load)
    {
        this->proc.clear();
        file_2_vector(config_proc.c_str(), content);
        vector_str_move_to_map_str(this->proc, content);
        proc_last_load = now;
        content.clear();
        WHITE_LIST_DEBUG_MAP(this->proc, "white_list_proc");
    }

    time_t modu_last_change = MAX(path_getmtime(config_modu.c_str()), path_getctime(config_modu.c_str()));
    WHITE_LIST_DEBUG_PRINT("last_change:%lu, last_load:%lu, now:%lu\n", 
            modu_last_change, modu_last_load, now);
    if(0 == modu_last_change || modu_last_change > modu_last_load)
    {
        this->modu.clear();
        file_2_vector(config_modu.c_str(), content);
        vector_str_move_to_map_str(this->modu, content);
        modu_last_load = now;
        content.clear();
        WHITE_LIST_DEBUG_MAP(this->modu, "white_list_modu");
    }

    time_t host_last_change = MAX(path_getmtime(config_host.c_str()), path_getctime(config_host.c_str()));
    WHITE_LIST_DEBUG_PRINT("last_change:%lu, last_load:%lu, now:%lu\n", 
            host_last_change, host_last_load, now);
    if(0 == host_last_change || host_last_change > host_last_load)
    {
        this->host.clear();
        file_2_vector(config_host.c_str(), content);
        vector_str_move_to_map_str(this->host, content);
        host_last_load = now;
        content.clear();
        WHITE_LIST_DEBUG_MAP(this->host, "white_list_host");
    }
}

bool white_list::is_str_in_map(std::map<uint32_t, std::string> &map, const char *needle)
{
    size_t needle_len = strlen(needle);
    uint32_t hash = fast_hash(needle, needle_len);
    std::string &value = map[hash];
    if(value.size() > 0)
    {
        if(0 == strncmp(value.c_str(), needle, MIN(needle_len, value.size())))
        {
            WHITE_LIST_DEBUG_PRINT("%s\n", needle);
            return true;
        }
    }
    return false;
}

bool white_list::is_file_in_white_list(const char *file_name)
{
    return is_str_in_map(file, file_name);
}

bool white_list::is_proc_in_white_list(const char *proc_name)
{
    return is_str_in_map(proc, proc_name);
}

bool white_list::is_modu_in_white_list(const char *modu_name)
{
    return is_str_in_map(modu, modu_name);
}

bool white_list::is_host_in_white_list(const char *host_name)
{
    return is_str_in_map(host, host_name);
}
