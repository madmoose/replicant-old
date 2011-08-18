#ifndef BR_ENGINE_H
#define BR_ENGINE_H

#include "BRAVFrame.h"
#include "BRPtrRange.h"

#include <stdint.h>

typedef struct BREngine * BREngineRef;

BREngineRef BREngineCreate();

BRAVFrameRef BREngineGetFrame(BREngineRef aEngine);

BRPtrRangeRef BREngineGetResource(BREngineRef aEngine, const char *name);

#endif
