#ifndef BR_DATA_H
#define BR_DATA_H

#include <stdint.h>
#include <stdlib.h>

typedef struct BRData * BRDataRef;

BRDataRef BRDataCreate(size_t size);
BRDataRef BRDataCreateWithBytes(size_t size, uint8_t *bytes);
BRDataRef BRDataCreateWithData(BRDataRef aData);

uint8_t *BRDataGetBytes(BRDataRef aData);
size_t   BRDataGetSize(BRDataRef aData);

#endif
