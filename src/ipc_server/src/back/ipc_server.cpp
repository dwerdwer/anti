#include "libwebsockets_server_eventloop.h"
#include "libwebsockets.h"
#include "libwebsockets_server_module_defines.h"

#include "utils/utils_network.h"
#include "utils/utils_library.h"

#include <iostream>
#include <string>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <queue>
#include "IRpcMessage.h"
#include "RpcMessage.h"
#include "ipc_server.h"

const char g_p_format_str [] = "%s received ok";

struct ipc_server{
	lws_server_evloop_t *p_evloop;
};

typedef struct queue_ele
{
	char *p_out_buffer;
	uint32_t out_len;
	queue_ele()
	{
		p_out_buffer = NULL;
		out_len = 0;
	}
}queue_ele_t;

typedef struct conn_params
{
	pthread_spinlock_t lock;
	bool b_isclosable;

	char *p_output_buffer;
	uint32_t output_length;

	char *p_input_buffer;
	uint32_t input_length;
	
	std::queue<queue_ele_t*> *p_output_queue;
}conn_params_t;

struct connection
{
	void *p_token;
	conn_params_t *p_conn_params;

	connection()
	{
		p_token = NULL;
		p_conn_params = NULL;
	}
};

typedef struct wt_params
{
	// Note: use 'conn_params_t' to store the params of each connection,
    // in order to handle async request synchronously.
	std::queue<connection_t> conns_queue;
	char* p_last_output_buffer;
	pthread_spinlock_t lock;
}wt_params_t;

class callback : public callback_base
{
public:
	virtual void notify(connect_t p_conn_params, server_notify_reason_t reason,
			peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len);
	virtual void sync_request(void *p_conn_params,
			const char *p_input_data, uint32_t input_data_len,
			OUT const char **pp_output_data, OUT uint32_t *p_output_data_len);
	virtual void async_request(void *p_thread_params,connect_t p_conn_params,  void *p_token,
			const char *p_data, uint32_t data_len);

	virtual void async_result(void *p_thread_params, OUT void **pp_token,
			OUT const char **p_data, OUT uint32_t *p_data_len);

	virtual bool is_closable(void *p_conn_params);

	virtual void* connection_create(void);

	virtual void connection_destroy(void *p_conn_params);
public:
	inline void set_notify(notify_cbk_t notify_cbk)
	{
		m_notify_cbk = notify_cbk;
	}
	inline notify_cbk_t get_notify()
	{
		return m_notify_cbk;
	}

private:
	notify_cbk_t m_notify_cbk;
};

typedef callback callback_t;

bool callback::is_closable(void *p_conn_params)
{
	return false;
	/*
	conn_params_t *p_params = (conn_params_t*)p_conn_params;
	return p_params->b_isclosable;
	*/
}

void *callback::connection_create(void)
{
//	conn_params_t *p_result = new conn_params_t;
	 void *p_result = malloc(sizeof(conn_params_t));
	 if (NULL != p_result)
	 {
		 conn_params_t *p = (conn_params_t*)p_result;
		 p->b_isclosable = false;
		 p->p_output_buffer = NULL;
		 p->output_length = 0;
		 p->p_input_buffer = NULL;
		 p->input_length = 0;
		 p->p_output_queue = new std::queue<queue_ele_t*>;
		 pthread_spin_init(&(p->lock), 0);
     }
    return (void*)p_result;
}

void callback::connection_destroy(void *p_conn_params)
{
	return;
	conn_params_t* p_param = (conn_params_t*)p_conn_params;

	if (p_param != NULL)
	{
		if(p_param->p_output_queue != NULL)
		{
			
			delete p_param->p_output_queue;
		}
		if (p_param->p_output_buffer != NULL)
			free(p_param->p_output_buffer);
		if (p_param->p_input_buffer != NULL)
			free(p_param->p_input_buffer);

		//pthread_spin_destroy(&(p_param->lock));
	//	free(p_param);
	}
}

void callback::notify(void *p_conn_params, server_notify_reason_t reason, 
		peer_addr_t *p_peer_addr, const char *p_info, uint32_t info_len)
{
	switch(reason)
	{
		case SERVER_NOTIFY_CONNECTION_ESTABLISHED:
			lwsl_notice("client %s:%d, connection established, info: %s\n",
					p_peer_addr->address, p_peer_addr->port, p_info);
			break;
		case SERVER_NOTIFY_GET_CLIENT_ADDR_FAILED:
			lwsl_notice("get client addr failed, info: %s\n", p_info);
			break;
		case SERVER_NOTIFY_CONNECTION_CLOSED:
			{
				conn_params_t *p_params = (conn_params_t*)p_conn_params;
				pthread_spin_lock(&(p_params->lock));
				p_params->b_isclosable = true;
				pthread_spin_unlock(&(p_params->lock));
				lwsl_notice("client %s:%d, connection closed, info: %s\n",
						p_peer_addr->address, p_peer_addr->port, p_info);
			}
			break;
		case SERVER_NOTIFY_RECEIVE_DATA:
			lwsl_notice("clietn %s:%d, receive data, size: %u\n",
					p_peer_addr->address, p_peer_addr->port, info_len);
			break;
		case SERVER_NOTIFY_RECEIVE_DATA_IGNORED:
			lwsl_notice("client %s:%d, receive data ignored,  size: %u\n",
					p_peer_addr->address, p_peer_addr->port, info_len);
			break;
		case SERVER_NOTIFY_RECEIVE_SELF_BUFFER_FULL:
			lwsl_notice("client %s:%d, receive self buffer full \n",
					p_peer_addr->address, p_peer_addr->port);
			break;
		case SERVER_NOTIFY_RECEIVE_SELF_REQUEST_ERROR:
			lwsl_notice("client %s:%d, receive self request error \n",
					p_peer_addr->address, p_peer_addr->port);
			break;
		case SERVER_NOTIFY_RECEIVE_WORKER_ERROR:
			lwsl_notice("client %s:%d, receive worker error \n",
					p_peer_addr->address, p_peer_addr->port);
			break;
		case SERVER_NOTIFY_WORKER_ASYNC_REQUEST_ERROR:
			lwsl_notice("client %s:%d, worker async request error\n",
					p_peer_addr->address, p_peer_addr->port);
			break;
		case SERVER_NOTIFY_WORKER_ASYNC_RESULT_ERROR:
			lwsl_notice("client %s:%d, worker async result error\n",
					p_peer_addr->address, p_peer_addr->port);
			break;
		case SERVER_NOTIFY_SEND_DATA:
			lwsl_notice("client %s:%d, send datas, size: %u\n",
					p_peer_addr->address, p_peer_addr->port, info_len);
			break;
		case SERVER_NOTIFY_SEND_DATA_ERROR:
			lwsl_notice("client %s:%d, send data error, size: %u\n",
					p_peer_addr->address, p_peer_addr->port, info_len);
			break;
		default:
			break;

	}
}
//处理同步
static int32_t prepare_output_buffer(conn_params_t *p_params, uint32_t output_length) 
{
	int32_t result = 0;
	if (p_params->output_length != 0)
	{
		char *p_temp = (char*)malloc(p_params->output_length + output_length);
		if(NULL != p_temp)
		{
			if (p_params->p_output_buffer != NULL)
			{	
				memmove(p_temp, p_params->p_output_buffer, p_params->output_length);
				free(p_params->p_output_buffer);
			}
			p_params->p_output_buffer = p_temp;
			p_params->output_length = p_params->output_length + output_length;
		}
		else
		{
			result = -1;
		}
	}
	else
	{
		char *p_temp = (char*)malloc(output_length);
		if (NULL != p_temp)
		{
			if (p_params->p_output_buffer != NULL)
				free(p_params->p_output_buffer);
			p_params->p_output_buffer = p_temp;
			p_params->output_length = output_length;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}
//处理异步
static int32_t prepare_output_buffer(queue_ele_t *p_ele, uint32_t output_length)
{
	int32_t result = 0;
	if (p_ele->out_len != 0)
	{
		char *p_temp = (char*)malloc(p_ele->out_len + output_length);
		if (NULL != p_temp)
		{
			if (p_ele->p_out_buffer != NULL)
			{
				memmove(p_temp, p_ele->p_out_buffer, p_ele->out_len);
				free(p_ele->p_out_buffer);
			}
			p_ele->p_out_buffer = p_temp;
			p_ele->out_len = p_ele->out_len + output_length;
		}
		else
		{
			result = -1;
		}
	}
	else
	{
		char *p_temp = (char*)malloc(output_length);
		if (NULL != p_temp)
		{
			if(p_ele->p_out_buffer != NULL)
				free(p_ele->p_out_buffer);
			p_ele->p_out_buffer = p_temp;
			p_ele->out_len = output_length;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}
/*
   static int32_t prepare_input_buffer(queue_ele_t* p_ele, uint32_t input_length)
   {
   int32_t result = 0;
   if ( p_ele->in_len != 0)
   {
   char *p_temp = (char*)malloc(p_ele->in_len + input_length);
   if (NULL != p_temp)
   {
   memmove(p_temp, p_ele->p_in_buffer, p_ele->in_len);
   if (p_ele->p_in_buffer != NULL)
   free(p_ele->p_in_buffer);
   p_ele->p_in_buffer = p_temp;
   p_ele->in_len = p_ele->in_len + input_length;
   }
   else
   {
   result = -1;
   }
   }
   else
   {
   char *p_temp = (char*)malloc(input_length);
   if (NULL != p_temp)
   {
   if (p_ele->p_in_buffer != NULL)
   free(p_ele->p_in_buffer);
   p_ele->p_in_buffer = p_temp;
   p_ele->in_len = input_length;
   }
   else
   {
   result = -1;
   }
   }
   return result;
   }
 */
static int32_t prepare_input_buffer(conn_params_t* p_conn_params, uint32_t input_length)
{
	int32_t result = 0;
	if (p_conn_params->input_length != 0)
	{
		char *p_temp = (char*)malloc(p_conn_params->input_length + input_length);
		if (NULL != p_temp)
		{
			memmove(p_temp, p_conn_params->p_input_buffer, p_conn_params->input_length);
			if (p_conn_params->p_input_buffer != NULL)
				free(p_conn_params->p_input_buffer);
			p_conn_params->p_input_buffer = p_temp;
			p_conn_params->input_length = p_conn_params->input_length + input_length;
		}
		else
		{
			result = -1;
		}
	}
	else
	{
		char *p_temp = (char*)malloc(input_length);
		if (NULL != p_temp)
		{
			if (p_conn_params->p_input_buffer != NULL)
				free(p_conn_params->p_input_buffer);
			p_conn_params->p_input_buffer = p_temp;
			p_conn_params->input_length = input_length;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}
void callback::sync_request(void *p_conn_params,
		const char *p_input_data, uint32_t input_data_len,
		const char **pp_output_data, uint32_t *p_output_data_len)
{

	conn_params_t *p_params = (conn_params_t*)p_conn_params;

	int msg_len;
	IRpcMessage *msg = NewInstance_IRpcMessage((const unsigned char*)p_input_data, input_data_len, &msg_len);

	io_params_t io_params;
	io_params.sync_flag = 0;
	m_notify_cbk(msg, &io_params);

	delete msg;

	std::string str_data = io_params.sync_params.msg->Encode();
	io_params.sync_params.release_msg(io_params.sync_params.msg);
	uint32_t response_length = (uint32_t)str_data.length();
	if (0 == prepare_output_buffer((conn_params_t*)p_conn_params, response_length))
	{
		*p_output_data_len = response_length;
		memcpy(p_params->p_output_buffer,str_data.c_str(), p_params->output_length);
		*pp_output_data = p_params->p_output_buffer;
	}
	else
	{
		lwsl_err("Handle sync request failed. Data: %s ",
				msg->to_String().c_str());
	}
}

void callback::async_request(void *p_thread_params, void* p_conn_params,
		void *p_token, const char *p_data, uint32_t data_len)
{
	wt_params_t *p_params = (wt_params_t*)p_thread_params;


	connection_t *p_connection = (connection_t*)malloc(sizeof(connection_t));
	p_connection->p_token = p_token;
	p_connection->p_conn_params = (conn_params_t*)p_conn_params;

	pthread_spin_lock(&(p_connection->p_conn_params->lock));	
	if (0 == prepare_input_buffer(p_connection->p_conn_params, data_len))
	{
		memcpy(p_connection->p_conn_params->p_input_buffer + p_connection->p_conn_params->input_length - data_len, p_data, data_len);

		int msg_len;
		IRpcMessage* msg = NewInstance_IRpcMessage((const unsigned char*)p_connection->p_conn_params->p_input_buffer, p_connection->p_conn_params->input_length, &msg_len);
		if (msg_len < 0 && msg_len > -100)
		{
			printf("data length is no enough! %d\n", msg_len);
		}
		else if (msg_len < -100)
		{
			printf("data is invalid \n");
			if (p_connection->p_conn_params->p_input_buffer)
			{
				free(p_connection->p_conn_params->p_input_buffer);
				p_connection->p_conn_params->p_input_buffer = NULL;
				p_connection->p_conn_params->input_length = 0;
			}
		}
		else
		{ // normal 
			p_connection->p_conn_params->input_length -= msg_len;
			if (p_connection->p_conn_params->input_length  == 0)
			{
				if (p_connection->p_conn_params->p_input_buffer!= NULL)
				{
					free(p_connection->p_conn_params->p_input_buffer);
					p_connection->p_conn_params->p_input_buffer = NULL;	
				}
			}
			else
			{
				char* p_temp = (char*)malloc(p_connection->p_conn_params->input_length);
				memcpy(p_temp, p_connection->p_conn_params->p_input_buffer + msg_len, p_connection->p_conn_params->input_length);
				free(p_connection->p_conn_params->p_input_buffer);
				p_connection->p_conn_params->p_input_buffer = p_temp;
			}
			io_params_t io_params;
			io_params.sync_flag = 1;
			io_params.async_params.p_conn = p_connection;
			io_params.async_params.p_wt_param = p_params;
			printf("p_conn address is %p\n", p_connection);
			m_notify_cbk(msg, &io_params);

			delete msg;
		}
	}
	else
	{
		lwsl_err("Handle async request failed.");
	}

	pthread_spin_unlock(&(p_connection->p_conn_params->lock));
}

void callback::async_result(void *p_thread_params, OUT void **pp_token,
		OUT const char **p_data, OUT uint32_t *p_data_len)
{
	/*
	*pp_token = NULL;
	*p_data = NULL;
	*p_data_len = 0;
	*/
	wt_params_t *p_params = (wt_params_t*)p_thread_params;

	if (p_params->p_last_output_buffer != NULL)
	{
		free(p_params->p_last_output_buffer);
		p_params->p_last_output_buffer = NULL;
	}

	pthread_spin_lock(&(p_params->lock));
	if (p_params->conns_queue.empty())
	{
		*pp_token = NULL;
		*p_data = NULL;
		*p_data_len = 0;
		pthread_spin_unlock(&(p_params->lock));
		return ;
	}
	else
	{
		connection_t *p_conn = &(p_params->conns_queue.front());
		conn_params_t *p_conn_params = p_conn->p_conn_params;
		pthread_spin_lock(&(p_conn_params->lock));
		p_params->conns_queue.pop();
		if (p_conn_params->b_isclosable)
		{
			*pp_token = NULL;
			*p_data = NULL;
			*p_data_len = 0;
			pthread_spin_unlock(&(p_params->lock));
			return ;
		}

		std::queue<queue_ele_t*> *output_queue = p_conn_params->p_output_queue;
		if (output_queue->empty())
		{
			*pp_token = NULL;
			*p_data = NULL;
			*p_data_len = 0;
			pthread_spin_unlock(&(p_conn_params->lock));
			pthread_spin_unlock(&(p_params->lock));
			return;
		}

		queue_ele_t *p_ele = NULL;
		p_ele = output_queue->front();
		output_queue->pop();

		*p_data = p_ele->p_out_buffer;
		*p_data_len = p_ele->out_len;
		*pp_token = p_conn->p_token;
		p_params->p_last_output_buffer = p_ele->p_out_buffer;
		delete p_ele;	
		pthread_spin_unlock(&(p_conn_params->lock));

	}

	pthread_spin_unlock(&(p_params->lock));
}

typedef struct 
{
	lws_server_evloop_t *p_event_loop;
	int32_t blocking_timeout;
}thread_arg_t;

void *thread_run_evloop(void *param)
{
	thread_arg_t *p_thread_arg = (thread_arg_t*)param;
	launch_server_evloop(p_thread_arg->p_event_loop, p_thread_arg->blocking_timeout);
	pthread_exit(NULL);
}

static lws_server_evloop_t*  run_evloop(lws_server_wt_t *p_wt, callback_t* p_callback,  uint32_t port)
{
	lws_server_evloop_info_t info;
	info.p_callback = p_callback;
	info.protocol_group = PROTOCOL_GROUP_BASED_ON_RAW_SOCKET;
	info.working_thread = *p_wt;
	info.p_iface = NULL;
	info.port = (uint16_t)port;
	info.hint_max_sending_size = 4096;

	lws_server_evloop_t *p_evloop = create_server_evloop(&info);
	thread_arg_t *p_arg_list = (thread_arg_t*)malloc(sizeof(thread_arg_t)); //这段内存在哪释放
	if (NULL == p_arg_list)
	{
		printf("malloc thread_arg_t err\n");
		return NULL;
	}
	p_arg_list->blocking_timeout = 500;
	p_arg_list->p_event_loop = p_evloop;

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, thread_run_evloop, (void*)p_arg_list);

	return p_evloop;
}

static int32_t initialize_wt_params(wt_params_t **p_wt_params_ptrs,
		uint8_t working_thread_count)
{
	int32_t result = 0;
	uint8_t i = 0;
	for(; i < working_thread_count; ++i)
	{
		wt_params_t *p_temp = new wt_params_t;
		//	wt_params_t *p_temp = (wt_params_t*)malloc(sizeof(wt_params_t));
		if (NULL != p_temp)
		{
			p_temp->p_last_output_buffer = NULL;
			pthread_spin_init(&(p_temp->lock), 0);

			p_wt_params_ptrs[i] = p_temp;
		}
		else
		{
			result = -1;
			break;
		}
	}
	return result;
}

static wt_params_t **create_wt_params_ptrs(uint8_t working_thread_count)
{
	wt_params_t **pp_result = NULL;
	wt_params_t **pp_temp = 
		(wt_params_t**)malloc(working_thread_count * sizeof(wt_params_t*));
	if(NULL != pp_temp &&
			0 == initialize_wt_params(pp_temp, working_thread_count))
	{
		pp_result = pp_temp;
	}
	return pp_result;
}

static void destroy_wt_params_ptrs(wt_params_t **pp_wt_params_ptrs,
		uint8_t working_thread_count)
{
	uint8_t i = 0;
	for (; i < working_thread_count; ++i)
	{
		//		pthread_spin_destroy(&(pp_wt_params_ptrs[i]->lock));
		delete(pp_wt_params_ptrs[i]);
	}
	free(pp_wt_params_ptrs);
}

static lws_server_evloop_t* run_evloop_with_working_thread(uint8_t working_thread_count, uint32_t port, callback_t* p_callback)
{
	lws_server_evloop_t *p_evloop = NULL;
	wt_params_t **pp_wt_params = create_wt_params_ptrs(working_thread_count);
	if(NULL != pp_wt_params)
	{
		lws_server_wt_t wt;
		wt.pp_params = (void**)pp_wt_params;
		wt.count = (uint8_t)working_thread_count;

		p_evloop = run_evloop(&wt, p_callback, port);
		//	destroy_wt_params_ptrs(pp_wt_params, working_thread_count);
	}
	return p_evloop;
}

static lws_server_evloop_t* run_evloop_without_working_thread(uint32_t port, callback_t* p_callback)
{
	lws_server_evloop_t *p_evloop = NULL;
	lws_server_wt_t wt;
	wt.pp_params = (void**)NULL;
	wt.count = 0;
	p_evloop = run_evloop(&wt, p_callback, port);
	return p_evloop;
}

LIB_PUBLIC ipc_server_t* start_ipc_server(uint32_t wt_count, uint32_t port, notify_cbk_t notify_cbk)
{

	ipc_server_t *p_ipc_server = (ipc_server_t*)malloc(sizeof(ipc_server_t*));
	if (NULL == p_ipc_server)
		return NULL;
	callback_t* p_callback = new callback_t();
	p_callback->set_notify(notify_cbk);
	lws_server_evloop_t *p_evloop = NULL;
	if (wt_count == 0)
		p_evloop = run_evloop_without_working_thread(port, p_callback);
	else
		p_evloop = run_evloop_with_working_thread((uint8_t)wt_count, port, p_callback);

	p_ipc_server->p_evloop = p_evloop;

	return p_ipc_server;

}

LIB_PUBLIC void stop_ipc_server(ipc_server_t *p_ipc_server)
{
	stop_server_evloop(p_ipc_server->p_evloop, true);
	destroy_server_evloop(p_ipc_server->p_evloop);
}


LIB_PUBLIC uint32_t assign_data(wt_params_t *p_th_params,connection_t *p_conn, IRpcMessage* msg)
{
	uint32_t result = 0;

	//创建 connection_t push 进 thread_params, result 中删除
	if (p_th_params == NULL || p_conn == NULL)
		return 0;


	std::string str_data = msg->Encode();

	uint32_t response_length = (uint32_t)str_data.length();
	queue_ele_t *ele = new queue_ele_t;
	if (NULL == ele)
		return 0;
	prepare_output_buffer(ele, response_length);
	memcpy(ele->p_out_buffer,str_data.c_str(), ele->out_len);

	if (p_conn->p_conn_params && p_conn->p_conn_params->b_isclosable == false)
	{
		pthread_spin_lock(&(p_conn->p_conn_params->lock));
		if (p_conn->p_conn_params)
		{
			p_conn->p_conn_params->p_output_queue->push(ele);
		}
		pthread_spin_unlock(&(p_conn->p_conn_params->lock));
		pthread_spin_lock(&(p_th_params->lock));
		p_th_params->conns_queue.push(*p_conn);
		pthread_spin_unlock(&(p_th_params->lock));
	}
	else
	{
		delete ele;
		result = -1;
	}
	return result;
}

