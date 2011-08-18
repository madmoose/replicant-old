#ifndef BR_VQA_READER_H
#define BR_VQA_READER_H

#include "BRCommon.h"
#include "BRAVFrame.h"
#include "BRPtrRange.h"
#include "BRUtils.h"

typedef struct BRVQAReader * BRVQAReaderRef;

BRVQAReaderRef BRVQAReaderOpen(BRPtrRangeRef r);

void BRVQAReaderClose(BRVQAReaderRef aVQAReaderRef);

uint16_t BRVQAReaderGetFrameCount(BRVQAReaderRef aVQAReaderRef);

BOOL BRVQAReaderReadFrame(BRVQAReaderRef aVQAReader, unsigned int aFrameNumber);

BRAVFrameRef BRVQAReaderGetAVFrame(BRVQAReaderRef aVQAReader);

BOOL BRVQAReaderGetLoop(BRVQAReaderRef aVQAReader, int aLoop, int *aFrameBegin, int *aFrameEnd);

#endif
