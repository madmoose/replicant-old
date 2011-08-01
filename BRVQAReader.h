#ifndef VQA_HC_H
#define VQA_HC_H

#include "BRPtrRange.h"

typedef struct BRVQAReader * BRVQAReaderRef;

BRVQAReaderRef BRVQAReaderOpen(BRPtrRangeRef r);

void BRVQAReaderClose(BRVQAReaderRef aVQAReaderRef);

#endif
