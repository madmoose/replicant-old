#ifndef BR_RETAIN_H
#define BR_RETAIN_H

typedef struct BRRetainable_s * BRRetainableRef;

typedef void (*BRRetainableDeallocFuncRef)(void *a);

typedef struct BRRetainable_s
{
	unsigned int retainCount;
	BRRetainableDeallocFuncRef deallocFunc;
} BRRetainable;

void *BRRetain(void *);
void  BRRelease(void *);

void BRRetainableSetDeallocFunc(void *, BRRetainableDeallocFuncRef aDeallocFunc);

#endif
