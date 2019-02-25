#include "../Include/LinuxUtils.h"
#include <pthread.h>


HLOCK AV_CALLTYPE AVCreateLock()
{
    pthread_mutex_t* mtx = new pthread_mutex_t;

    pthread_mutex_init(mtx,NULL);

    return mtx;
}

void AV_CALLTYPE AVLock(HLOCK hLock)
{
   	pthread_mutex_lock( hLock );
}

void AV_CALLTYPE AVUnlock(HLOCK hLock)
{
    pthread_mutex_unlock( hLock );
}

void AV_CALLTYPE AVDisposeLock(HLOCK hLock)
{
	pthread_mutex_destroy( hLock );
	delete hLock;
}
