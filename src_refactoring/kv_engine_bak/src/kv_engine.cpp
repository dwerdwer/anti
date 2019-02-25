#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif /* _BSD_SOURCE */
#include <endian.h>

#include <map>
#include <string>

#include "IRpcClient.h"
#include "kv_engine.h"
#include "kv_engine_public.h"
#include "RpcMessage.h"
#include "RpcMessage.pb.h"

#define KEY_VALUE_SEPERATOR ":"
#define PAIR_SEPERATOR       ";"

#define HOST_NAME       "host"
#define CPU_NAME        "cpu"
#define MEMORY_NAME     "memory"
#define BASEBOARD_NAME  "baseboard"
#define SOUNDCARD_NAME  "sound"
#define VIDEOCARD_NAME  "video"

#define CMD_GET_HOST        "uname -n"
#define CMD_GET_CPU         "grep 'model name' /proc/cpuinfo | awk -F':' '{print $2}' | tail -1"
#define CMD_GET_MEMORY      "grep MemTotal /proc/meminfo | awk '{ print $2 }' | tail -1"
#define CMD_GET_BASEBOARD   "dmidecode -t baseboard | grep 'Product Name' | tail -1"
#define CMD_GET_SOUNDCARD   "dmidecode -t baseboard | grep Sound -a2 | grep Description | awk -F':' '{ print $2 }' | tail -1"
#define CMD_GET_VIDEOCARD   "dmidecode -t baseboard | grep Video -a2 | grep Description | awk -F':' '{ print $2 }' | tail -1"

#define AVSDK_CHECKSUM 0x123456789abcdef
static const uint64_t checksum_net = htobe64(AVSDK_CHECKSUM);

#ifndef verbose_print
#define verbose_print(...)\
    do {\
        fprintf(stderr, "[VERBOSE] ");\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)
#endif /* verbose_print */

#ifndef debug_print
#define debug_print(...)\
    do {\
        fprintf(stderr, "[  DEBUG] ");\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)
#endif /* debug_print */

#ifndef info_print
#define info_print(...)\
    do {\
        fprintf(stderr, "[   INFO] ");\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)
#endif /* info_print */


struct req_t
{
    int id;
    int type;
    std::string data;
};

class kv_engine : public IRpcNotify
{
    public:
        kv_engine(const char *ip = "127.0.0.1", uint16_t port = 7681);
        virtual ~kv_engine();
        int request_sync(req_t *req /*in*/, req_t *resp /*out*/);
        int set_notifier(notifier_t notifier = nullptr, void *p_param = nullptr);
        bool enable_disable_monitor(int typeinternal, const char *data, size_t datalen);

        /* for notify */
        virtual int NotifyInfo(IRpcConnection *hConnection, IRpcMessage *msg);
        virtual int OnDisconnected(IRpcConnection* hConnection, int error = 0);

        int NotifyKVEngine(IRpcConnection *hConnection, IRpcMessage *msg);
        bool CheckSum(IRpcConnection *hConnection, IRpcMessage *msg);

        IRpcConnection *hConnection;

        int id;
        std::string host;
        uint16_t port;

        notifier_t notifier;
        void *p_param;
};

kv_engine::~kv_engine()
{

}

kv_engine::kv_engine(const char *ip, uint16_t port)
{
    if (ip == nullptr)
    {
        ip = "127.0.0.1";
    }
    this->host.assign(ip);
    this->port = port;
    this->id = 0;
    this->set_notifier(nullptr, nullptr);
    this->hConnection = NULL;

    IRpcMessage *msg = New_IRpcMsg(RPC_AV_SDK_CMD, RPC_AV_SDK_SUB_CMD);
    msg->Add_IntValue(RPC_AV_SDK_REQ_ID, htonl(this->id++));
    msg->Add_IntValue(RPC_AV_SDK_REQ_TYPE, htonl(KV_ENGINE_MSG_TYPE_CENTER_AGENT_POST));
    msg->Add_BinValue(RPC_AV_SDK_CHECKSUM, std::string((const char *)&checksum_net, sizeof(uint64_t)));

    /* start a keep-alive connection */
    QueryNotify(this->host.c_str(), this->port, msg, this);
}

int kv_engine::NotifyInfo(IRpcConnection *hConnection, IRpcMessage *msg)
{
    verbose_print("%s\n", __func__);
    msg->Dump();
    if (this->hConnection == NULL)
    {
        this->hConnection = hConnection;
    }
    if (msg->Get_Cmd() == RPC_AV_SDK_CMD && msg->Get_SubCmd() == RPC_AV_SDK_SUB_CMD)
    {
        this->NotifyKVEngine(hConnection, msg);
    }
    else
    {
        info_print("%s (%d, %d), cmd || subcmd error\n",
                __func__, msg->Get_Cmd(), msg->Get_SubCmd());
    }
    Release_IRpcMsg(msg);
    return 0;
}

bool kv_engine::CheckSum(IRpcConnection *p_ipc_conn, IRpcMessage *pMsg)
{
    int sz = 0;
    const char *p = pMsg->Get_BinValue(RPC_AV_SDK_CHECKSUM, &sz);
    if (sz != sizeof(uint64_t) || p == NULL)
    {
        info_print("%s (%d, %d) checksum Get_BinValue error\n",
                __func__, pMsg->Get_Cmd(), pMsg->Get_SubCmd());
        return false;
    }
    uint64_t checksum = be64toh(*(const uint64_t *)p);
    if (checksum != AVSDK_CHECKSUM)
    {
        info_print("%s (%d, %d) checksum(0x%" PRIx64 ") error\n",
                __func__, pMsg->Get_Cmd(), pMsg->Get_SubCmd(), checksum);
        return false;
    }
    verbose_print("%s (%d, %d) checksum(0x%" PRIx64 ") success\n",
            __func__, pMsg->Get_Cmd(), pMsg->Get_SubCmd(), checksum);
    return true;
}

int kv_engine::NotifyKVEngine(IRpcConnection *hConnection, IRpcMessage *pMsg)
{
    if (!this->CheckSum(hConnection, pMsg))
    {
        return -1;
    }
    if (this->notifier == NULL)
    {
        verbose_print("%s no notifier set, ingore notify\n", __func__);
        return 0;
    }

    req_t resp;
    resp.id = pMsg->Get_IntValue(RPC_AV_SDK_REQ_ID);
    resp.type = pMsg->Get_IntValue(RPC_AV_SDK_REQ_TYPE);
    int sz = 0;
    const char *p = pMsg->Get_BinValue(RPC_AV_SDK_REQ_DATA, &sz);
    if (p && sz)
    {
        resp.data.assign(p, sz);
    }
    this->notifier(resp.data.c_str(), resp.data.size(), this->p_param);
    return 0;
}

int kv_engine::OnDisconnected(IRpcConnection *hConnection, int error)
{
    if(error)
    {
        info_print("%s errorno:%d\n", __func__, error);
    }
    else
    {
        verbose_print("%s errorno:%d\n", __func__, error);
    }
    hConnection->Stop();
    return 0;
}

int kv_engine::set_notifier(notifier_t notifier, void *p_param)
{
    if (notifier == NULL)
    {
        return -1;
    }
    this->notifier = notifier;
    this->p_param = p_param;
    return 0;
}

int kv_engine::request_sync(req_t *req /*in*/, req_t *resp /*out*/)
{
    int err = -1;

    /* write message */
    IRpcMessage *pMsg = New_IRpcMsg(RPC_AV_SDK_CMD, RPC_AV_SDK_SUB_CMD);
    pMsg->Add_IntValue(RPC_AV_SDK_REQ_ID, htonl(req->id));
    pMsg->Add_IntValue(RPC_AV_SDK_REQ_TYPE, htonl(req->type));
    if (req->data.size())
    {
        pMsg->Add_BinValue(RPC_AV_SDK_REQ_DATA, req->data);
    }
    pMsg->Add_BinValue(RPC_AV_SDK_CHECKSUM, std::string((const char *)&checksum_net, sizeof(uint64_t)));
    // QueryNotify("127.0.0.1", 7681, pMsg, (IRpcNotify *)this);
    // RPCCLIENT_API IRpcMessage* QueryData(IN const char* hostInfo, IN int port, IN IRpcMessage* msg, OUT int * pdwError);

    /* query */
    pMsg = QueryData(this->host.c_str(), this->port, pMsg, &err);
    if (err || pMsg == nullptr)
    {
        return err;
    }

    if (resp == nullptr)
    {
        delete pMsg;
        return err;
    }

    resp->id = ntohl(pMsg->Get_IntValue(RPC_AV_SDK_REQ_ID));
    resp->type = ntohl(pMsg->Get_IntValue(RPC_AV_SDK_REQ_TYPE));
    int sz = 0;
    const char *p = pMsg->Get_BinValue(RPC_AV_SDK_REQ_DATA, &sz);
    if (p && sz)
    {
        resp->data.assign(p, sz);
    }

    return err;
}

bool kv_engine::enable_disable_monitor(int typeinternal, const char *data, size_t datalen)
{
    verbose_print("%s typeinternal:%d, enable:%c\n", __func__, typeinternal, (*data)+'0');
    req_t req, resp;
    req.id = this->id++;
    req.type = typeinternal;
    req.data.assign(data, datalen);
    this->request_sync(&req, &resp);
    if (resp.data.size() && resp.data[0] == 0)
    {
        verbose_print("%s typeinternal:%d, result:%c ok\n", __func__, typeinternal, resp.data[0]+'0');
        return true;
    }

    /* debug error data */
    info_print("[");
    for (size_t i = 0; i < resp.data.size(); i++)
    {
        printf("%02x ", resp.data[i]);
    }
    putchar(']');
    putchar(10);

    info_print("%s typeinternal:%d, result:%c false\n", __func__, typeinternal, resp.data[0]+'0');
    return false;
}

__attribute__ ((visibility ("default"))) int32_t enable_monitor(kv_engine_t *p_engine, monitor_type_t type)
{
    verbose_print("%s typeinterface:%d\n", __func__, type);
    char enable = 1;
    int ret = type;

    if (type & MONITOR_TYPE_FILE)
    {
        if (p_engine->enable_disable_monitor(KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR, &enable, sizeof(enable)))
        {
            ret &= ~MONITOR_TYPE_FILE;
        }
    }
    if (type & MONITOR_TYPE_PROCESS)
    {
        if (p_engine->enable_disable_monitor(KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR, &enable, sizeof(enable)))
        {
            ret &= ~MONITOR_TYPE_PROCESS;
        }
    }
    if (type & MONITOR_TYPE_NETWORK)
    {
        if (p_engine->enable_disable_monitor(KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR, &enable, sizeof(enable)))
        {
            ret &= ~MONITOR_TYPE_NETWORK;
        }
    }
    return ret;
}

__attribute__ ((visibility ("default"))) int32_t disable_monitor(kv_engine_t *p_engine, monitor_type_t type)
{
    verbose_print("%s typeinterface:%d\n", __func__, type);
    char enable = 0;
    std::string res;
    int ret = type;

    if (type & MONITOR_TYPE_FILE)
    {
        if (p_engine->enable_disable_monitor(KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR, &enable, sizeof(enable)))
        {
            ret &= ~MONITOR_TYPE_FILE;
        }
    }
    if (type & MONITOR_TYPE_PROCESS)
    {
        if (p_engine->enable_disable_monitor(KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR, &enable, sizeof(enable)))
        {
            ret &= ~MONITOR_TYPE_PROCESS;
        }
    }
    if (type & MONITOR_TYPE_NETWORK)
    {
        if (p_engine->enable_disable_monitor(KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR, &enable, sizeof(enable)))
        {
            ret &= ~MONITOR_TYPE_NETWORK;
        }
    }
    return ret;
}

static void string_split(std::string str, std::string sep, std::vector<std::string> &v)
{
    size_t lpos = 0, rpos = 0;

    while (true)
    {
        if (lpos == str.size())
        {
            break;
        }
        rpos = str.find_first_of(sep, lpos);
        if (rpos == std::string::npos)
        {
            v.push_back(str.substr(lpos));
            break;
        }
        else
        {
            v.push_back(str.substr(lpos, rpos-lpos));
            lpos = rpos + 1;
        }
    }
}

__attribute__ ((visibility ("default"))) int32_t get_assets_info(kv_engine_t *p_engine, uint32_t *p_count, const asset_t **pp_assets)
{
    assert(p_engine);
    req_t req, resp;
    req.id = p_engine->id++;
    req.type = KV_ENGINE_MSG_TYPE_GET_ASSETS;
    verbose_print("%s start request_sync\n", __func__);
    if (p_engine->request_sync(&req, &resp))
    {
        info_print("%s request_sync error\n",
            __func__);
        return KV_ENGINE_MSG_TYPE_GET_ASSETS;
    }
    verbose_print("%s end request_sync\n", __func__);
    /* TODO: fill pp_assets from resp.data */
    std::vector<std::string> assets;
    string_split(resp.data, PAIR_SEPERATOR, assets);
    if (assets.size() == 0)
    {
        info_print("%s string_split(response.data:[%s]) -> 0 toks, error\n",
            __func__, resp.data.c_str());
        return KV_ENGINE_MSG_TYPE_GET_ASSETS;
    }

    asset_t *p_assets = (asset_t *)calloc(assets.size(), sizeof(asset_t));
    assert(p_assets);
    size_t count = 0;

    for (std::vector<std::string>::iterator itr = assets.begin();
        itr != assets.end(); ++itr)
    {
        std::vector<std::string> keyvalue;
        string_split(*itr, KEY_VALUE_SEPERATOR, keyvalue);
        if (keyvalue.size() != 2)
        {
            info_print("%s [%s] split by(%s) -> (%lu)toks, error\n",
                __func__, itr->c_str(), KEY_VALUE_SEPERATOR, keyvalue.size());
            continue;
        }
        p_assets[count].p_name = strdup(keyvalue[0].c_str());
        p_assets[count].p_description = strdup(keyvalue[1].c_str());
        assert(p_assets[count].p_name && p_assets[count].p_description);

        count++;
        verbose_print("%s [%s] [%s]\n",
            __func__, keyvalue[0].c_str(), keyvalue[1].c_str());
    }

    *pp_assets = p_assets;
    *p_count = count;

    return 0;
}

__attribute__ ((visibility ("default"))) int32_t set_notifier(kv_engine_t *p_engine, notifier_t notifier, void *p_param)
{
    assert(p_engine);
    return p_engine->set_notifier(notifier, p_param);
}

__attribute__ ((visibility ("default"))) kv_engine_t *initialize_engine()
{
    return new kv_engine_t;
}

__attribute__ ((visibility ("default"))) void finalize_engine(kv_engine_t *p_engine)
{
    assert(p_engine);
    delete p_engine;
}

__attribute__ ((visibility ("default"))) int upgrade_daemon(kv_engine_t *p_engine,
                    update_type_t type, uint32_t file_count, const char **pp_files)
{
    assert(p_engine);

    req_t req, resp;
    req.id = p_engine->id++;
    req.type = KV_ENGINE_MSG_TYPE_UPDATE_DAEMON;
    verbose_print("%s start request_sync\n", __func__);
    if (p_engine->request_sync(&req, &resp))
    {
        info_print("%s request_sync error\n",
            __func__);
        return KV_ENGINE_MSG_TYPE_UPDATE_DAEMON;
    }

    verbose_print("%s -> %s\n", __func__, resp.data.c_str());
    if (resp.data.size() && resp.data[0] == 0)
    {
        return 0;
    }
    return KV_ENGINE_MSG_TYPE_UPDATE_DAEMON;
}
