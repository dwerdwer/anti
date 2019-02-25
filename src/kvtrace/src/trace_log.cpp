#include "trace_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>

char g_sz_log_file[PATH_MAX];
pthread_spinlock_t g_spin_lock;

void init_log(const char *p_log_path)
{
    if (p_log_path != NULL)
        strcpy(g_sz_log_file, p_log_path);
    else
        return;

    // crate gloable lock
    pthread_spin_init(&g_spin_lock, 0);

    FILE *f = fopen(g_sz_log_file, "r+b");
    if (f != NULL)
    {
        fseek(f, 0, 2);
        long lSize = ftell(f);
        if (lSize > 10 * 1024 * 1024) {
            int len = 2 * 1024 * 1024;
            int offs = 0;
            char *buf = new char[len + 1];
            if (buf != NULL) {
                buf[len + 1] = 0;
                fseek(f , lSize - len, 0);
                fread(buf, 1, len, f);
                char* lp = strchr(buf, '\n');
                if (lp == NULL)
                    offs = len;
                else
                    offs = lp - buf;
                fseek(f, 0, 0);
                fwrite(buf + offs, 1, len - offs, f);
                ftruncate(fileno(f) , len - offs);
                delete []buf;
            }
        }
        fclose(f);
    }

}

void dbg_log(const char *fmt, ...)
{
   if (g_sz_log_file[0] == 0)
       return ;

   va_list ap;
   char buff[MAX_LOG_LEN];

   va_start(ap, fmt);
   vsnprintf(buff, MAX_LOG_LEN - 1, fmt, ap);
   va_end(ap);

   // lock
   pthread_spin_lock(&g_spin_lock);
   FILE *fp = fopen(g_sz_log_file, "at");
   if (fp != NULL){
       struct timeval sys_now;
       gettimeofday(&sys_now, NULL);

       char tmp_time[64] = {0};
       strftime(tmp_time, 64, "%Y-%m-%d %H:%M:%S", localtime(&sys_now.tv_sec));
//       snprintf(time_buf, sizeof(time_buf), "%s", tmp_time);

       fprintf(fp, "%s", tmp_time);
       fprintf(fp, "%s\n", buff);
       fclose(fp);

   }
   //unlock
   pthread_spin_unlock(&g_spin_lock);
}
