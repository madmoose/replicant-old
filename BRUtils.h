#ifndef BR_UTILS_H
#define BR_UTILS_H

#include "BRCommon.h"

#include <stddef.h>

void hexdump(void *data, size_t size);

typedef struct {
	unsigned int width;
	unsigned int height;
} BRSize;

BRSize BRSizeMake(unsigned int width, unsigned int height);
BOOL BRSizeIsEmpty(BRSize s);

#endif
