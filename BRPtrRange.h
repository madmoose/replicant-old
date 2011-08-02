#ifndef BR_PTR_RANGE
#define BR_PTR_RANGE

#include <stdint.h>
#include <stddef.h>

typedef struct BRPtrRange * BRPtrRangeRef;

BRPtrRangeRef BRPtrRangeCreate(uint8_t *begin, uint8_t *end);
BRPtrRangeRef BRPtrRangeCreateCopy(BRPtrRangeRef aPtrRange);

uint8_t *BRPtrRangeGetBegin(BRPtrRangeRef r);
uint8_t *BRPtrRangeGetEnd(BRPtrRangeRef r);
size_t  BRPtrRangeGetDistance(BRPtrRangeRef r);
size_t  BRPtrRangeGetAdvancement(BRPtrRangeRef r);

void BRPtrRangeSetEnd(BRPtrRangeRef r, uint8_t *end);
void BRPtrRangeSetDistance(BRPtrRangeRef r, size_t aDistance);

void BRPtrRangeAdvance(BRPtrRangeRef r, int aAdvancement);

uint8_t  BRPtrRangeReadByte(BRPtrRangeRef r);
uint16_t BRPtrRangeReadLE16(BRPtrRangeRef r);
uint16_t BRPtrRangeReadBE16(BRPtrRangeRef r);
uint32_t BRPtrRangeReadLE32(BRPtrRangeRef r);
uint32_t BRPtrRangeReadBE32(BRPtrRangeRef r);

uint8_t  BRPtrRangeReadByteAndAdvance(BRPtrRangeRef r);
uint16_t BRPtrRangeReadLE16AndAdvance(BRPtrRangeRef r);
uint16_t BRPtrRangeReadBE16AndAdvance(BRPtrRangeRef r);
uint32_t BRPtrRangeReadLE32AndAdvance(BRPtrRangeRef r);
uint32_t BRPtrRangeReadBE32AndAdvance(BRPtrRangeRef r);

#endif
