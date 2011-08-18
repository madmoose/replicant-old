#ifndef BR_AV_FRAME_H
#define BR_AV_FRAME_H

#include "BRData.h"
#include "BRUtils.h"

#include <stdint.h>

typedef struct BRAudioFrame * BRAudioFrameRef;
typedef struct BRVideoFrame * BRVideoFrameRef;
typedef struct BRAVFrame    * BRAVFrameRef;


/*
 * BRAudioFrame
 */

BRAudioFrameRef BRAudioFrameCreate(uint32_t aSampleCount, BRDataRef aData);

uint32_t BRAudioFrameGetSampleCount(BRAudioFrameRef aAudioFrame);
BRDataRef BRAudioFrameGetData(BRAudioFrameRef aAudioFrame);

/*
 * BRVideoFrame
 */

BRVideoFrameRef BRVideoFrameCreate(BRSize aSize, BRSize aStride, BRDataRef aData);

BRSize BRVideoFrameGetSize(BRVideoFrameRef aVideoFrame);
BRSize BRVideoFrameGetStride(BRVideoFrameRef aVideoFrame);
BRDataRef BRVideoFrameGetData(BRVideoFrameRef aVideoFrame);

/*
 * BRAVFrame
 */

BRAVFrameRef BRAVFrameCreate(BRAudioFrameRef aAudioFrame, BRVideoFrameRef aVideoFrame);

BRAudioFrameRef BRAVFrameGetAudioFrame(BRAVFrameRef aAVFrame);
BRVideoFrameRef BRAVFrameGetVideoFrame(BRAVFrameRef aAVFrame);


#endif
