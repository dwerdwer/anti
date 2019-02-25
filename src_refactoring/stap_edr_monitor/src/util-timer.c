#include <stdint.h>
#include <string.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "util-timer.h"

#define TIMER_CAPACITY 1024

struct timer_handle
{
    pthread_t           thread_id;
    struct timer_node  *head;
    pthread_spinlock_t  lock;
    int                 run;
    size_t              timers;
    size_t              capacity;
};

struct timer_node
{
    int                 fd;
    timer_callback_t    callback;
    void               *p_user_data;
    unsigned int        interval_ms;
    unsigned int        periodic_ms;
    size_t              count;
    struct timer_node  *next;
};

static void *_timer_thread(void * data);

timer_handle_t *timer_init(size_t maxtimers)
{
    timer_handle_t *p_handle = (timer_handle_t *)calloc(1, sizeof(timer_handle_t));
    if (p_handle == NULL)
    {
        return p_handle;
    }

    if (maxtimers == 0)
    {
        maxtimers = TIMER_CAPACITY;
    }

    p_handle->capacity = maxtimers;

    p_handle->run = 1;

    if (pthread_create(&p_handle->thread_id, NULL, _timer_thread, p_handle))
    {
        return p_handle;
    }

    pthread_spin_init(&p_handle->lock, PTHREAD_PROCESS_PRIVATE);

    return p_handle;
}

size_t start_timer(timer_handle_t *p_handle, unsigned int interval_ms, unsigned int periodic_ms, size_t count, timer_callback_t p_callback, void *p_user_data)
{
    struct timer_node * new_node = NULL;
    struct itimerspec new_value;

    if (p_handle == NULL || p_callback == NULL)
    {
        return 0;
    }

    new_node = (struct timer_node *)malloc(sizeof(struct timer_node));

    if (new_node == NULL)
    {
        return 0;
    }

    new_node->callback      = p_callback;
    new_node->p_user_data   = p_user_data;
    new_node->interval_ms   = interval_ms;
    new_node->periodic_ms   = periodic_ms;
    new_node->count         = count;

    new_node->fd = timerfd_create(CLOCK_REALTIME, 0);

    if (new_node->fd == -1)
    {
        free(new_node);
        return 0;
    }

    new_value.it_value.tv_sec  = interval_ms / 1000;
    new_value.it_value.tv_nsec = interval_ms % 1000 * 1000000;

    new_value.it_interval.tv_sec  = periodic_ms / 1000;
    new_value.it_interval.tv_nsec = periodic_ms % 1000 * 1000000;    

    timerfd_settime(new_node->fd, 0, &new_value, NULL);

    /*Inserting the timer node into the list*/
    pthread_spin_lock(&p_handle->lock);
    if (p_handle->timers == p_handle->capacity)
    {
        free(new_node);
        new_node = NULL;
    }
    else
    {
        new_node->next = p_handle->head;
        p_handle->head = new_node;
    }
    pthread_spin_unlock(&p_handle->lock);

    return (size_t)new_node;
}

void stop_timer(timer_handle_t *p_handle, size_t timer_id)
{
    struct timer_node * tmp = NULL;
    struct timer_node * node = (struct timer_node *)timer_id;

    if (p_handle == NULL || node == NULL)
    {
        return;
    }

    pthread_spin_lock(&p_handle->lock);

    close(node->fd);

    if (node == p_handle->head)
    {
        p_handle->head = p_handle->head->next;
    }

    tmp = p_handle->head;

    while (tmp && tmp->next != node) 
    {
        tmp = tmp->next;
    }

    if (tmp && tmp->next)
    {
        tmp->next = tmp->next->next;
    }

    if (node)
    {
        free(node);
    }

    pthread_spin_unlock(&p_handle->lock);

}

void timer_deinit(timer_handle_t *p_handle)
{
    p_handle->run = 0;

    pthread_join(p_handle->thread_id, NULL);

    p_handle->thread_id = 0;

    while (p_handle->head) 
    {
        stop_timer(p_handle, (size_t)p_handle->head);
    }
}

static struct timer_node * _get_timer_from_fd(timer_handle_t *p_handle, int fd)
{
    struct timer_node * tmp = p_handle->head;

    while (tmp)
    {
        if (tmp->fd == fd) return tmp;

        tmp = tmp->next;
    }
    return NULL;
}

void *_timer_thread(void * data)
{
    timer_handle_t *p_handle = (timer_handle_t *)data;

    struct pollfd *ufds = (struct pollfd *)malloc(p_handle->capacity * sizeof(struct pollfd));
    assert(ufds);

    int iMaxCount = 0;
    struct timer_node * tmp = NULL;
    int read_fds = 0, i, s;
    uint64_t exp;

    while (p_handle->run)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        iMaxCount = 0;
        memset(ufds, 0, p_handle->capacity * sizeof(struct pollfd));

        pthread_spin_lock(&p_handle->lock);
        tmp = p_handle->head;
        while (tmp && iMaxCount < (int)p_handle->capacity)
        {
            ufds[iMaxCount].fd = tmp->fd;
            ufds[iMaxCount].events = POLLIN;
            iMaxCount++;

            tmp = tmp->next;
        }
        pthread_spin_unlock(&p_handle->lock);

        read_fds = poll(ufds, iMaxCount, 100);

        if (read_fds <= 0)
        {
            continue;
        }

        for (i = 0; i < iMaxCount; i++)
        {
            if (!(ufds[i].revents & POLLIN))
            {
                continue;
            }

            s = read(ufds[i].fd, &exp, sizeof(uint64_t));

            if (s != sizeof(uint64_t))
            {
                continue;
            }

            int is_del = 0;

            pthread_spin_lock(&p_handle->lock);

            tmp = _get_timer_from_fd(p_handle, ufds[i].fd);

            if (!tmp || !tmp->callback)
            {
                pthread_spin_unlock(&p_handle->lock);
                continue;
            }

            while (exp--)
            {
                tmp->callback((size_t)tmp, tmp->p_user_data);
                if (tmp->count == 0 || tmp->count == 1)
                {
                    is_del = 1;
                    break;
                }
                else
                {
                    tmp->count--;
                }
            }
            pthread_spin_unlock(&p_handle->lock);

            if (is_del)
            {
                stop_timer(p_handle, (size_t)tmp);
            }
        }
    }

    free(ufds);

    return NULL;
}
