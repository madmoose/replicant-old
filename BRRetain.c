#include "BRRetain.h"

#include <stdlib.h>

#include <stdio.h>

void *BRRetain(void *p)
{
	BRRetainable *n = (BRRetainable*)p;
	n->retainCount++;

	return n;
}

void BRRelease(void *p)
{
	BRRetainable *n = (BRRetainable*)p;

	// Like free, BRRelease can be called on null
	if (!p)
		return;


	if (n->retainCount == 0)
	{
		if (n->deallocFunc)
			n->deallocFunc(n);
		else
			free(n);
		return;
	}

	n->retainCount--;
}

void BRRetainableSetDeallocFunc(void *p, BRRetainableDeallocFuncRef aDeallocFunc)
{
	BRRetainable *n = (BRRetainable*)p;
	n->deallocFunc = aDeallocFunc;
}
