#ifndef BR_CONTROLLER_H
#define BR_CONTROLLER_H

#include "BREngine.h"
#include "BRFrame.h"

typedef struct BRController * BRControllerRef;

BRControllerRef BRControllerCreateVQAPlayer(BREngineRef aEngine, const char *aVQAName);

BRControllerRef BRControllerSequenceCreate(BREngineRef aEngine);
BOOL BRControllerSequenceAddController(BRControllerRef aSequence, BRControllerRef aController);

BOOL BRControllerGetAVFrame(BRControllerRef aController, BRAVFrame *aFrame);

void BRControllerVQAPlayerSetLeadInLoop(BRControllerRef aController, int aLeadInLoop);
void BRControllerVQAPlayerSetMainLoop(BRControllerRef aController, int aMainLoop);

#endif
