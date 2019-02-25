#ifndef __UTIL_TIMER_H__
#define __UTIL_TIMER_H__
#include <stdlib.h>

typedef void (*timer_callback_t)(size_t timer_id, void * user_data);

typedef struct timer_handle timer_handle_t;

/*  */
timer_handle_t *timer_init(size_t maxtimers);

size_t start_timer(timer_handle_t *p_handle, unsigned int interval_ms, unsigned int periodic_ms, size_t count, timer_callback_t handler, void *p_user_data);
void stop_timer(timer_handle_t *p_handle, size_t timer_id);
void timer_deinit(timer_handle_t *p_handle);

#endif /* __UTIL_TIMER_H__ */