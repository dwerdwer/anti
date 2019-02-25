#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <csignal>
#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "ipc_server.h"

bool g_run = true;
void *g_p_token = NULL;

void sig_handler(int sig)
{
	if (sig == SIGINT)
	{
		printf("catch interrupt\n");
		exit(0);
	}

}
class CRpcServer : public IRpcServer
{
public:
    virtual void    OnConnect(IRpcConnection* p_ipc_conn)
    {
        // TODO:  user determine stor this p_ipc_conn or not
		// if store then , OnClose must call p_ipc_conn->Release() to release the buffer where this pointer to point

    }

    virtual void    OnReceive(IRpcConnection* p_ipc_conn, IRpcMessage* msg)
    {
        msg->Dump();
		IRpcMessage *repmsg = NewInstance_IRpcMessage(2,2222);
		repmsg->Add_StringValue(2, "test_responses##@");
		repmsg->Add_IntValue(3, 495);
/*
        static int index = 0;
        while(1) {

            index = index + 1;
            char buffer[1024] = {0};

            sprintf(buffer, "****************************index = %d", index);

            IRpcMessage *reqmsg = NewInstance_IRpcMessage(2,2222);
            reqmsg->Add_StringValue(2,buffer);
            reqmsg->Add_IntValue(3,495);
            reqmsg->Dump();
//            if( index > 3 )
  //              continue;
            if (p_ipc_conn)
                p_ipc_conn->SendResponse(m_p_evloop, reqmsg);
            else
                break;
            delete reqmsg;
            usleep(1000);
        }
        */
        delete repmsg;
    }

    virtual void OnClose(IRpcConnection* p_ipc_conn)
    {
        //TODO:  if user already stored this p_ipc_conn,
        // user must call p_ipc_conn->Release() and remove this pointer
		// else do nothing!!!
        // p_ipc_conn->Release();
    //    p_ipc_conn = NULL;

        p_ipc_conn = NULL;
        printf("#######################");
    }


    virtual void ClearInstParam(IRpcConnection* p_ipc_conn)
    {}

    virtual void SetEvloop(void *p_evloop)
    {
        m_p_evloop = p_evloop;
    }

private:
    void* m_p_evloop;
};

int main(void)
{
	signal(SIGINT, sig_handler);

    CRpcServer *p_rpcserver = new CRpcServer;
	ipc_server_t* p_server = start_ipc_server(5, 7681, p_rpcserver);
	if (NULL == p_server)
	{
		printf("start ipc server failed!\n");
		return -1;
	}

	while(g_run)
	{
        //sleep();
	}

	stop_ipc_server(p_server);
}


