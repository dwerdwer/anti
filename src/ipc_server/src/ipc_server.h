#ifndef _IPC_SERVER_H_
#define _IPC_SERVER_H_
#endif

#include <stddef.h>
#include <stdio.h>

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

typedef struct ipc_server ipc_server_t;

#define MAX_INSTPARAMS  16

class IRpcConnection
{
public:
    virtual void    AddRef() = 0;
    virtual void     Release() = 0;

    virtual void*   GetInstParam(int index = 0) = 0;
    virtual void    SetInstParam(void* pClient, int index = 0) = 0;

//	virtual void 	OnConnect() = 0;
//    virtual void    OnReceive(const char* p_data, uint32_t data_len) = 0;
    virtual int     SendResponse(void *p_evloop, IRpcMessage *msg) = 0;
//    virtual void    OnClose() = 0;
};

class IRpcServer
{
public:
    virtual void    OnConnect(IRpcConnection* p_ipc_conn) = 0;

    virtual void    OnReceive(IRpcConnection* p_ipc_conn, IRpcMessage* msg) = 0;

    virtual void    OnClose(IRpcConnection* p_ipc_conn) = 0;

    virtual void    ClearInstParam(IRpcConnection* p_ipc_conn) = 0;

    virtual void    SetEvloop(void *evloop) = 0;

};


#ifdef __cplusplus
extern "C" {

	ipc_server_t* start_ipc_server(uint32_t wt_count, uint32_t port, IRpcServer* p_rpc_server);

	void stop_ipc_server(ipc_server_t *p_ipc_server);
}
#endif
