#ifndef QUEUE_HHH
#define QUEUE_HHH

#include <stddef.h>
#include <stdint.h>

/////////////////////////////////////////////////////////
// Queue Stub Part
/////////////////////////////////////////////////////////
// Definition of QueueElementType could be changed to any type 
// in source code, before compiling code.
typedef void* QueueElementType;

////////////////////////////////////////////////////////
// Queue Declaration Part
////////////////////////////////////////////////////////

struct Queue;

// Return NULL if creation failed.
// Actual count of available elements is 'size' - 1.
struct Queue* createQueue(uint32_t size);

void destroyQueue(struct Queue* pQueue);

// Return 0 if enqueue is successful, otherwise it is failed.
int enqueue(struct Queue* pQueue, QueueElementType data);

// Return 0 if dequeue is successful, otherwise it is failed.
// Element data is filled in parameter 'pData'.
int dequeue(struct Queue* pQueue, QueueElementType* pData);

int isQueueEmpty(struct Queue* pQueue);

#endif

