#pragma once
#include <string>

#define SEPERATOR "="
#define DAEMON_KEY      "daemon"
#define VIRUSLIB_KEY    "viruslib"

class VersionManager
{
public:
    virtual ~VersionManager();
    virtual bool init(std::string);
    virtual bool check_upgrade(std::string);
    virtual std::string curr_version();

    virtual std::string get_key() = 0;
    virtual bool compare(std::string) = 0;
    virtual bool is_version_legal(std::string) = 0;
private:
    std::string _ver;
};

class DaemonVersionManager : public VersionManager
{
public:
    virtual std::string get_key();
    virtual bool compare(std::string);
    virtual bool is_version_legal(std::string);
};


class VirusLibVersionManager : public VersionManager
{
public:
    virtual std::string get_key();
    virtual bool compare(std::string);
    virtual bool is_version_legal(std::string);
};
