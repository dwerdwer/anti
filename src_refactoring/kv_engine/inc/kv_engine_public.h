/* this header file is used to commuticate between client with server */

#ifndef _KVENGINEXXXXX_
#define _KVENGINEXXXXX_

#pragma once

/* client commuticate with server with RPC_MESSAGE_PROTOCOL */
#include "RpcMessage.pb.h"


/* client commuticate with server with these enum vars */
enum
{
    KV_ENGINE_MSG_TYPE_MIN = 1234,
    KV_ENGINE_MSG_TYPE_ENABLE_FILE_MONITOR,
    KV_ENGINE_MSG_TYPE_ENABLE_PROC_MONITOR,
    KV_ENGINE_MSG_TYPE_ENABLE_NET_MONITOR,
    KV_ENGINE_MSG_TYPE_GET_ASSETS,
    KV_ENGINE_MSG_TYPE_CENTER_AGENT_POST,
    KV_ENGINE_MSG_TYPE_UPDATE_DAEMON,
    KV_ENGINE_MSG_TYPE_UPDATE_VIRUSLIB,
    KV_ENGINE_MSG_TYPE_UPDATE_DAEMON_VIRUSLIB,
    KV_ENGINE_MSG_TYPE_RPCSRV,
    KV_ENGINE_MSG_TYPE_MAX
};

#endif
