#pragma once

#include <string>
#include <sys/types.h>

int makedir(std::string, mode_t);
int makedir_p(std::string, mode_t);
bool str_endswith(const char *, size_t, const char *, size_t);
bool str_endswith(const char *, const char *);
bool str_endswith(std::string &, std::string &);
bool str_endswith(std::string &, const char *);
std::string str_replace(std::string, const char *, const char *);
std::string str_replace(const char *, const char *, const char *);
std::string path_format(std::string);
std::string tok(const std::string str, const std::string sep, size_t index);
std::string tok_key(const std::string str, const std::string sep);
std::string tok_value(const std::string str, const std::string sep);
