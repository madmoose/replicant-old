#include "BRRetain.h"

#include <stdlib.h>

#include <stdio.h>

void BRRetain(void *p)
{
	BRRetainable *n = (BRRetainable*)p;
	n->retainCount++;
}

void BRRelease(void *p)
{
	// Like free, BRRelease can be called on null
	if (!p)
		return;

	BRRetainable *n = (BRRetainable*)p;

	if (n->retainCount == 0)
	{
		free(n);
		return;
	}

	n->retainCount--;
}
