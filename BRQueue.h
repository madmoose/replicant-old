#ifndef BR_QUEUE_H
#define BR_QUEUE_H

typedef struct BRQueue * BRQueueRef;

BRQueueRef BRQueueCreate();

void BRQueueEnqueue(BRQueueRef aQueue, void *aRetainable);

void *BRQueueGetHead(BRQueueRef aQueue);
void  BRQueueDequeue(BRQueueRef aQueue);

#endif
