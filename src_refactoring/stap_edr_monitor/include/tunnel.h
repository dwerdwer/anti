#ifndef __TUNNEL_H__
#define __TUNNEL_H__
#include "sys_task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TUNNEL_INPUT_METHOD_APPEND  1
#define TUNNEL_INPUT_METHOD_UPDATE  2

void tunnel_input(sys_task_t *task);

sys_task_t *tunnel_output();








#ifdef __cplusplus
}
#endif

#endif /* __TUNNEL_H__ */
