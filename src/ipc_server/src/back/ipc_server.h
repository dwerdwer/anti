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

typedef struct conn_params conn_params_t;

typedef struct connection connection_t;

typedef struct wt_params wt_params_t;


typedef void(*release_msg_cbk_t)(IN IRpcMessage* msg);

typedef struct response_data
{
	IRpcMessage* msg;
	release_msg_cbk_t release_msg;
}response_data_t;

typedef struct io_params
{
	int sync_flag; 

	struct sync_params_t 
	{
		IRpcMessage* msg;
		release_msg_cbk_t release_msg;
	} sync_params;

	struct async_params_t
	{
		connection_t *p_conn;
		wt_params_t  *p_wt_param;
	}async_params;
}io_params_t;

typedef void(*notify_cbk_t)(IN IRpcMessage* msg , OUT io_params_t* io_params/*, OUT const IRpcMessage* msg*/);

#ifdef __cplusplus
extern "C" {

	ipc_server_t* start_ipc_server(uint32_t wt_count, uint32_t port, notify_cbk_t notify_cbk);

	void stop_ipc_server(ipc_server_t *p_ipc_server);
	uint32_t assign_data(wt_params_t *p_th_params, connection_t* p_conn, IRpcMessage* msg);
}
#endif
