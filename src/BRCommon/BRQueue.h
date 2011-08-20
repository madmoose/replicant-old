#ifndef BR_QUEUE_H
#define BR_QUEUE_H

#include <stdlib.h>

typedef struct BRQueue * BRQueueRef;

BRQueueRef BRQueueCreate();

void BRQueueEnqueue(BRQueueRef aQueue, void *aRetainable);

void *BRQueueGetHead(BRQueueRef aQueue);
void  BRQueueDequeue(BRQueueRef aQueue);

size_t BRQueueGetLength(BRQueueRef aQueue);

#endif
