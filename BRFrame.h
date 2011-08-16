#ifndef BR_FRAME_H
#define BR_FRAME_H

#include "BRData.h"
#include "BRUtils.h"

#include <stdint.h>

typedef struct
{
	BRSize    size;
	BRSize    stride;
	BRDataRef data;
} BRVideoFrame;

typedef struct
{
	uint32_t  samples;
	BRDataRef data;
} BRAudioFrame;

typedef struct
{
	BRVideoFrame video;
	BRAudioFrame audio;
} BRAVFrame;

#endif
