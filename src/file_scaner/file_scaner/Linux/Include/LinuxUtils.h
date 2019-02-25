#pragma  once

#include <TypeDef.h>

#ifndef MAX_PATH
#define MAX_PATH 	1024
#endif // MAX_PATH


typedef pthread_mutex_t* HLOCK;

HLOCK AV_CALLTYPE AVCreateLock();
void AV_CALLTYPE AVLock(HLOCK hLock);
void AV_CALLTYPE AVUnlock(HLOCK hLock);
void AV_CALLTYPE AVDisposeLock(HLOCK hLock);

#include <Module.h>

typedef enum
{
	OP_READONLY = 0,
	OP_READWRITE,
	OP_CREATE,
} EOPMODE;


IFileEx* FileOpen(const char* path, EOPMODE opMode=OP_READONLY);
IFileEx* FileCreateTemp();
