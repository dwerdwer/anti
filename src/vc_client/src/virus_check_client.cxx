
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/stat.h>

#include "vc_client_log.h"
#include "vc_client_defines.h"
#include "vc_client_function.h"

#include "RpcMessage.h"
#include "utils/utils_library.h"

static const char *g_message_types[] = {"UStorgae_Event", "MSG_TYPE_VIRUSSCAN"}; 

#define MAX_RESULT_BUF_LEN 4096

static char* get_file_buf(const char *file_path, size_t *p_buf_size)
{
    FILE *rfp = NULL;
    if ((rfp = fopen(file_path, "rb")) == NULL)
        return NULL;

    fseek(rfp, 0, SEEK_END);
    size_t buf_size = ftell(rfp);
    fseek(rfp, 0, SEEK_SET);

    char *file_buf = (char*)malloc(buf_size);

    if(file_buf)
    {
        *p_buf_size = buf_size;
        fread(file_buf, 1, buf_size, rfp);
    }
    fclose(rfp);

    return file_buf;
}

static bool need_send(module_t *p_module, const char *file_path)
{
    bool result = true;
    // If file_path exists
    if(0 == access(file_path, F_OK))
    {
        if(0 != strcmp(p_module->current_path.c_str(), file_path))
            p_module->current_path = file_path;
        else return false;
    }
    else return false;
    
    struct stat file_st;  

    memset(&file_st, 0, sizeof(struct stat)); 
    stat(file_path, &file_st);

    switch(S_IFMT & file_st.st_mode) // file type  
    {  
    case S_IFREG: break;
    case S_IFDIR: result = false; break;
    case S_IFSOCK: break;
    case S_IFLNK: break;
    case S_IFBLK: break;
    case S_IFCHR: break;
    case S_IFIFO: break;
    default: break;
    }  
    if(result)
    {
        switch(S_IRWXU & file_st.st_mode) // permission 
        {  
        case S_IRUSR|S_IWUSR|S_IXUSR: break; // rwx 
        case S_IWUSR: result = false; break; // write only 
        case S_IRUSR: result = false; break; // read only 
        case S_IRUSR|S_IWUSR: break;  
        default:  break;  
        }
    } 
    return result;
}

LIB_PUBLIC module_t *create(uint32_t category, notify_scheduler_t notifier, 
                            void *p_params, uint32_t arg_count, const char **p_args)
{
    if(NULL == p_args || arg_count < 2)
        return NULL;

    module *p_result = new module;

    if (NULL != p_result)
    {
        p_result->category = category;
        p_result->notifier = notifier;
        p_result->p_params = p_params;

        const char *rpc_host = p_args[0];
        int16_t rpc_port = (int16_t)atoi(p_args[1]);

        p_result->current_path = "";
        p_result->p_recdata_queue = new recdata_queue_t;
        p_result->p_send_params = new send_params_t;

        p_result->p_send_params->sync_params.p_result_buffer = new char [MAX_RESULT_BUF_LEN] ();
        p_result->p_send_params->sync_params.result_buffer_length = MAX_RESULT_BUF_LEN;

        p_result->p_rpc_client = create_rpc_client(rpc_host, rpc_port);

        vc_client_log(p_result, "receive host = %s port = %d", rpc_host, rpc_port);
    }
    return p_result;
}

LIB_PUBLIC void get_inputted_message_type(module_t *p_module, 
            const char ***const ppp_inputted_message_types, uint32_t *p_message_type_count)
{
    *ppp_inputted_message_types = g_message_types;

    *p_message_type_count = sizeof(g_message_types) / sizeof(g_message_types[0]);
}

LIB_PUBLIC module_state_t run(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    member_t front_member;
    while(true)
    {
        if(p_module->p_recdata_queue->empty())
            sleep(1);
        else
        {
            front_member = p_module->p_recdata_queue->front();

            // TODO: 发送到其他模块
            debug_print("file path: %s receive info: %s\n", 
                        front_member.first.c_str(), front_member.second.c_str());

            p_module->p_recdata_queue->pop();
        }
    }
    return MODULE_OK;	
}

LIB_PUBLIC module_state_t stop(module_t *p_module)
{
    if (NULL == p_module)
        return MODULE_ERROR;

    return MODULE_OK;
}

LIB_PUBLIC void destroy(module_t *p_module)
{
    if (NULL != p_module)
    {
        if(p_module->p_send_params)
        {
            if(p_module->p_send_params->sync_params.p_result_buffer)
                delete [] p_module->p_send_params->sync_params.p_result_buffer;

            delete p_module->p_send_params;
        }
        if(p_module->p_recdata_queue)
            delete p_module->p_recdata_queue; 

        if(p_module->p_rpc_client)
            destroy_rpc_client(p_module->p_rpc_client);

        delete p_module;
        p_module = NULL;
    }
}

LIB_PUBLIC module_data_t* assign(module_t *p_module, const module_data_t *p_data, bool is_sync)
{
    if (NULL == p_module || NULL == p_data) 
        return NULL;

    const char *p_message_type = NULL;

    uint32_t message_type_len = 0;

    if(0 !=  get_module_data_property(p_data, g_p_message_id, &p_message_type, &message_type_len))
        return NULL;

    // from usb monitor
    if(0 == strcmp(g_message_types[0], p_message_type)) 
    {
        const char *num_str; uint32_t num_str_size = 0;

        if(0 == get_module_data_property(p_data, "UStorgae_Event_Num", &num_str, &num_str_size)) 
        {
            for(int i = *(int *)num_str; i > 0; i--)
            {
                int path_flag = -1;

                const char *flag_str; uint32_t flag_size = 0;

                const char *file_path; uint32_t file_path_len = 0;

                if(0 == get_module_data_property(p_data, "UStorage_Flag", &flag_str, &flag_size)) {
                    path_flag = *(int *)flag_str;
                }
                else{
                    // vc_client_log(p_module, "%s: get_module_data_property(%s) failed", __func__, "USBSTORAGE_EVENT");
                    break;
                }
                if(0 == get_module_data_property(p_data, "UStorage_Data", &file_path, &file_path_len)) 
                {
                    if(0 == path_flag) {  
                        // process type
                    }
                    else if(1 == path_flag){
                        // file type 
                    }
                    if(need_send(p_module, file_path))
                    {
                        vc_client_log(p_module, "%s send to server file path: %s", "USBSTORAGE_EVENT", file_path);
                        /* Send file_path to vc_server */
                        if(0 == send_to_server(p_module->p_rpc_client, RPC_ECHO, 
                                               RPC_ECHO_TEST, file_path, file_path_len, p_module->p_send_params))
                        {
                            member_t member = std::make_pair(file_path, p_module->p_send_params->sync_params.p_result_buffer);
                            p_module->p_recdata_queue->push(member);
                        }
                        else
                            debug_print("%s send_to_server error\n", "USBSTORAGE_EVENT");

                        memset(p_module->p_send_params->sync_params.p_result_buffer, 0, 
                               p_module->p_send_params->sync_params.result_buffer_length);
                    }
                } 
            } 
        }
    }
    // from file monitor
    else if(0 == strcmp(g_message_types[1], p_message_type)) 
    {
        const char *file_path;
        uint32_t file_path_len;

        const char *path_message_key = "MONITOR_PATH_MESSAGE";
        if(0 == get_module_data_property(p_data, path_message_key, &file_path, &file_path_len))
        {
            if(need_send(p_module, file_path))
            {
                vc_client_log(p_module, "%s send to vc_server file path: %s", path_message_key, file_path);
                /* Send file_path to vc_server */
                if(0 == send_to_server(p_module->p_rpc_client, RPC_ECHO, 
                                       RPC_ECHO_TEST, file_path, file_path_len, p_module->p_send_params))
                {
                    member_t member = std::make_pair(file_path, p_module->p_send_params->sync_params.p_result_buffer);
                    p_module->p_recdata_queue->push(member);
                }
                else
                    debug_print("%s send_to_server error\n", path_message_key);

                memset(p_module->p_send_params->sync_params.p_result_buffer, 0, 
                       p_module->p_send_params->sync_params.result_buffer_length);
            }
        }
        else {  } // do nothing 
    }
    return NULL;
}
