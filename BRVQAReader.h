#ifndef VQA_HC_H
#define VQA_HC_H

#include "BRCommon.h"
#include "BRPtrRange.h"
#include "BRUtils.h"

typedef struct BRVQAReader * BRVQAReaderRef;

BRVQAReaderRef BRVQAReaderOpen(BRPtrRangeRef r);

void BRVQAReaderClose(BRVQAReaderRef aVQAReaderRef);

uint16_t BRVQAReaderGetFrameCount(BRVQAReaderRef aVQAReaderRef);
BRSize   BRVQAReaderGetFrameSize(BRVQAReaderRef aVQAReaderRef);

BOOL BRVQAReaderReadFrame(BRVQAReaderRef aVQAReader, unsigned int aFrameNumber);

uint8_t *BRVQAReaderGetFrame(BRVQAReaderRef aVQAReader);

#endif
