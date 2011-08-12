#include "BRQueue.h"

#include "BRRetain.h"

#include <assert.h>
#include <stdlib.h>

struct BRQueue
{
	struct BRQueuedItem *head;
	struct BRQueuedItem *tail;
	struct BRQueuedItem *freeListHead;
};

struct BRQueuedItem
{
	struct BRQueuedItem *next;
	void                *retainable;
};

BRQueueRef BRQueueCreate()
{
	BRQueueRef queue = calloc(1, sizeof(struct BRQueue));
	return queue;
}

void BRQueueEnqueue(BRQueueRef aQueue, void *aRetainable)
{
	BRRetain(aRetainable);

	struct BRQueuedItem *qi;
	if (aQueue->freeListHead)
	{
		qi = aQueue->freeListHead;
		aQueue->freeListHead =  aQueue->freeListHead->next;
		qi->next = qi->retainable = 0;
	}
	else
	{
		qi = calloc(1, sizeof(struct BRQueuedItem));
		assert(qi);
	}

	qi->retainable = aRetainable;

	if (!aQueue->head)
		aQueue->head = aQueue->tail = qi;
	else
	{
		aQueue->tail->next = qi;
		aQueue->tail = qi;
	}
}

void *BRQueueGetHead(BRQueueRef aQueue)
{
	if (!aQueue->head)
		return 0;

	return aQueue->head->retainable;
}

void BRQueueDequeue(BRQueueRef aQueue)
{
	struct BRQueuedItem *front = aQueue->head;
	assert(front);

	BRRelease(front->retainable);
	front->retainable = 0;

	aQueue->head = front->next;
	front->next = aQueue->freeListHead;
	aQueue->freeListHead = front;
}
