
#ifndef UTILS_SIGNAL_HHH
#define UTILS_SIGNAL_HHH

#include "utils_bool.h"

#include <stdint.h>

int32_t ignore_signal(int32_t signal, boolean_t is_once);

typedef void (*signal_handler_t)(int32_t signal, void *p_param_1, void *p_param_2);

int32_t handle_signal(int32_t signal, signal_handler_t handler, 
        boolean_t is_once, boolean_t is_resumable);

int32_t restore_signal_handler_to_default(int32_t signal);

#endif

