#ifndef BR_RETAIN_H
#define BR_RETAIN_H

typedef void (*BRRetainableDeallocFuncRef)(BRRetainableRef);

typedef struct
{
	unsigned int retainCount;
	BRRetainableDeallocFuncRef deallocFunc;
} BRRetainable;

void *BRRetain(void *);
void  BRRelease(void *);

void BRRetainableSetDeallocFunc(void *, BRRetainableDeallocFuncRef aDeallocFunc);

#endif
