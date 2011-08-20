#ifndef BR_ENGINE_H
#define BR_ENGINE_H

#include "BRCommon/BRCommon.h"

#include "BRAVFrame.h"

#include <stdint.h>

typedef struct BREngine * BREngineRef;

BREngineRef BREngineCreate();

BRAVFrameRef BREngineGetFrame(BREngineRef aEngine);

BRPtrRangeRef BREngineGetResource(BREngineRef aEngine, const char *name);

#endif
