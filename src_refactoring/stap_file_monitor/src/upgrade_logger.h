#pragma once
#include <string>
#include "upgrade_interface.h"

class CommonLogger
{
public:
    CommonLogger();
    ~CommonLogger();
    virtual void set_module_info(module_info_t *);
    virtual void set_prefix(const char *);
    virtual void log(const char *fmt, ...);
private:
    module_info_t *p_info;
    std::string prefix;
};

extern CommonLogger g_logger;