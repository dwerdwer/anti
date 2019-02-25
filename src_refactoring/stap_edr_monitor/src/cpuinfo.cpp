#include "cpuinfo.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static char                 g_init = 0;
static struct cpu_info_     g_cpu_info;
static pthread_spinlock_t   g_lock;

__attribute__((constructor))
static void lock_constructor ()
{
    if ( pthread_spin_init ( &g_lock, 0 ) != 0 )
    {
        exit ( 1 );
    }
}

__attribute__((destructor))
static void lock_destructor ()
{
    if ( pthread_spin_destroy ( &g_lock ) != 0 )
    {
        exit ( 3 );
    }
}

static int cpu_info_init()
{
    FILE *p = fopen(CPU_INFO_PATH, "r");
    if(!p)  return -1;
    char line[100];
    int len;
    do{
        if(!fgets(line, sizeof(line), p))  break;
        len = strlen(line);
        if(len == sizeof(line)-1)       continue;
        if(strstr(line, "cpu cores") && strrchr(line, ':'))
                g_cpu_info.cpu_cores = atoi(strrchr(line, ':')+1);
        /*TODO: any other info */

    }while(1);
    return 0;
}

const struct cpu_info_ *get_cpu_info()
{
    pthread_spin_lock(&g_lock);
    if(!g_init)
    {
        if(cpu_info_init())
            exit(-1);
        g_init = 1;
    }
    pthread_spin_unlock(&g_lock);

    return &g_cpu_info;
}

int get_cpu_cores()
{
    return get_cpu_info()->cpu_cores;
}
