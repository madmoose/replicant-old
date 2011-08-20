#ifndef BR_MIXFILE_H
#define BR_MIXFILE_H

#include "BRCommon/BRCommon.h"

#include <stddef.h>
#include <stdint.h>

typedef struct BRMixFile * BRMixFileRef;

BRMixFileRef BRMixFileOpen(const char *aFilename);

void BRMixFileClose(BRMixFileRef aMixFile);

const char *BRMixFileGetName(BRMixFileRef aMixFile);

uint16_t BRMixFileGetResourceCount(BRMixFileRef aMixFile);

BRPtrRangeRef BRMixFileGetResourceRangeByIndex(BRMixFileRef aMixFile, uint16_t aIndex);

BRPtrRangeRef BRMixFileGetResourceRangeByName(BRMixFileRef aMixFile, const char *aResourceName);

#endif
