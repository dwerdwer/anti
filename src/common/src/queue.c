#include "queue.h"

#include <stdlib.h>


////////////////////////////////////////////////////////
// Queue Definition Part
////////////////////////////////////////////////////////

///////////////////////////////
// Private Implementation
///////////////////////////////

struct Queue
{
	QueueElementType* m_pData;
	uint32_t head;
	uint32_t tail;
	uint32_t size;
};

static uint32_t nextIndex(uint32_t currentIndex, uint32_t modulo)
{
    return (currentIndex + 1) % modulo;
}

static int isQueueFull(struct Queue* pQueue)
{
    int result = -1;
    if (nextIndex(pQueue->tail, pQueue->size) == pQueue->head)
    {
        result = 0;
    }
    return result;
}

///////////////////////////////
// Public Interfaces
///////////////////////////////

struct Queue* createQueue(uint32_t size)
{
    struct Queue* pResult = (struct Queue*)malloc(sizeof(struct Queue));
    pResult->m_pData = (QueueElementType*)malloc(sizeof(QueueElementType) * size);
    pResult->head = 0;
    pResult->tail = 0;
    pResult->size = size;
    return pResult;
}

void destroyQueue(struct Queue* pQueue)
{
    if (NULL != pQueue)
    {
        free(pQueue->m_pData);
        free(pQueue);
    }
}

int enqueue(struct Queue* pQueue, QueueElementType data)
{
    int result = -1;
    if (NULL != pQueue && (0 != isQueueFull(pQueue)))
    {
        pQueue->m_pData[pQueue->tail] = data;
        pQueue->tail = nextIndex(pQueue->tail, pQueue->size);
        result = 0;
    }
    return result;
}

int isQueueEmpty(struct Queue* pQueue)
{
    int result = -1;
    if (pQueue->head == pQueue->tail)
    {
        result = 0;
    }
    return result;
}

int dequeue(struct Queue* pQueue, QueueElementType* pData)
{
    int result = -1;
    if (NULL != pQueue && (0 != isQueueEmpty(pQueue)))
    {
        *pData = pQueue->m_pData[pQueue->head];
        pQueue->head = nextIndex(pQueue->head, pQueue->size);
        result = 0;
    }
    return result;
}


