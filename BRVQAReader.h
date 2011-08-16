#ifndef VQA_HC_H
#define VQA_HC_H

#include "BRCommon.h"
#include "BRFrame.h"
#include "BRPtrRange.h"
#include "BRUtils.h"

typedef struct BRVQAReader * BRVQAReaderRef;

BRVQAReaderRef BRVQAReaderOpen(BRPtrRangeRef r);

void BRVQAReaderClose(BRVQAReaderRef aVQAReaderRef);

uint16_t BRVQAReaderGetFrameCount(BRVQAReaderRef aVQAReaderRef);

BOOL BRVQAReaderReadFrame(BRVQAReaderRef aVQAReader, unsigned int aFrameNumber);

BRAVFrame BRVQAReaderGetAVFrame(BRVQAReaderRef aVQAReader);

BOOL BRVQAReaderGetLoop(BRVQAReaderRef aVQAReader, int aLoop, int *aFrameBegin, int *aFrameEnd);

#endif
