#ifndef __STATISER_PROC_H__
#define __STATISER_PROC_H__

#include <inttypes.h>
#include <map>
#include <string>

#include "proclist.h"

struct simple_proc{
    simple_proc() : pid(0) {}
    simple_proc(uint64_t id, std::string n, unsigned long long tm_strt, unsigned long long tm_end):
        pid(id), name(n), starttime(tm_strt), endtime(tm_end) {}

    uint64_t pid;
    std::string name;
    unsigned long long starttime;
    unsigned long long endtime;
};

class proc_record {
public:
    proc_record();
    void update(PSCtx *ctx);
//     simple_proc get_proc_by_pid(uint64_t pid);
private:
    time_t boot_time;
    long clock_ticks;

    std::map<uint64_t, simple_proc> procs;
};

#endif /* __STATISER_PROC_H__ */
