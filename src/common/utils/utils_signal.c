
#include "utils_signal.h"

#include <stddef.h>

#include <signal.h>

/////////////////////////////////////////////////////////////////
// Private Implementations
/////////////////////////////////////////////////////////////////
typedef void (*sigaction_handler_t)(int32_t, siginfo_t *, void *);


/////////////////////////////////////////////////////////////////
// Public Interfaces
/////////////////////////////////////////////////////////////////
int32_t ignore_signal(int32_t signal, boolean_t is_once)
{
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    act.sa_flags |= (YES == is_once) ? SA_RESETHAND : 0;
    
    return sigaction(signal, &act, NULL);
}

int32_t handle_signal(int32_t signal, signal_handler_t handler, 
        boolean_t is_once, boolean_t is_resumable)
{
    struct sigaction act;
    act.sa_sigaction = (sigaction_handler_t)handler;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = SA_SIGINFO;
    act.sa_flags |= (YES == is_once) ? SA_RESETHAND : 0;
    act.sa_flags |= (YES == is_resumable) ? SA_RESTART : 0;
    
    return sigaction(signal, &act, NULL);
}

int32_t restore_signal_handler_to_default(int32_t signal)
{
    struct sigaction act;
    act.sa_handler = SIG_DFL;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    
    return sigaction(signal, &act, NULL);
}

