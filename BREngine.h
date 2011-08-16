#ifndef BR_ENGINE_H
#define BR_ENGINE_H

#include "BRFrame.h"
#include "BRPtrRange.h"

#include <stdint.h>

typedef struct BREngine * BREngineRef;

BREngineRef BREngineCreate();

BRAVFrame BREngineGetFrame(BREngineRef aEngine);

BRPtrRangeRef BREngineGetResource(BREngineRef aEngine, const char *name);

#endif
