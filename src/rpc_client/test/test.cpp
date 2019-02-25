#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "IRpcClient.h"

class CRpcNotify : public IRpcNotify
{
public:
	//通知消息
	virtual int NotifyInfo(IRpcConnection* hConnection, IRpcMessage* msg)
	{
		bool isclose = false;

    	// msg->Dump();
		Release_IRpcMsg(msg);

		IRpcMessage* resMsg = New_IRpcMsg(2,3);
		resMsg->Add_StringValue(4, "client response");

//		hConnection->SendResponse(resMsg);
		if (isclose)
		{
			hConnection->Stop();
			return -1;
		}
		else
		{
			return 0;
		}
	}

	//连接断开通知：
	virtual int OnDisconnected(IRpcConnection* hConnection, int error = 0)
	{
		hConnection->Stop();
		return -1;
	}
};

CRpcNotify Notify7681;

pid_t get_ThreadId()
{
    return syscall(SYS_gettid);
}

void* run_thread(void *p_params)
{
    CRpcNotify CNotify;
    IRpcMessage *msg = New_IRpcMsg(RPC_ECHO, RPC_ECHO_TEST);
    msg->Add_StringValue(RPCKEY_CHAR, "ceshi");
    pid_t pid = get_ThreadId();
    msg->Add_IntValue(RPCKEY_INT, pid);

    QueryNotify("127.0.0.1", 7681, msg, (IRpcNotify*)&CNotify);

    while(true)
        sleep(1);
}

int main(int argc, char *argv[])
{
    uint32_t thread_count = 0;
    int opt;
    while((opt = getopt(argc, argv, "c:")) != -1)
    {
        switch(opt){
            case 'c':
                thread_count = atoi(optarg);
                printf("option: %c thread_count: %d\n", opt, thread_count);
                break;
            case '?':
                printf("Unknown option: %c\n", (char)(optopt));
                return -1;
                break;
            case ':':
                printf("option needs a value \n");
                return -1;
        }
    }

/*
    for (uint32_t i = 0; i < thread_count; i++)
    {
        pthread_t ntid;
        pthread_create(&ntid, NULL, run_thread, NULL);
    }
*/
    for (uint32_t i = 0; i < thread_count; i++)
    {
        IRpcMessage *msg = New_IRpcMsg(RPC_ECHO, RPC_ECHO_TEST);
        msg->Add_StringValue(RPCKEY_CHAR, "ceshi");
        msg->Add_IntValue(RPCKEY_INT, i);

        QueryNotify("127.0.0.1", 7681, msg, (IRpcNotify*)&Notify7681);
    }
	getchar();

	puts("Stop client...\n");

	return 0;
}






