#ifndef BR_MAPFILE_H
#define BR_MAPFILE_H

#include <stddef.h>

typedef struct BRMappedFile * BRMappedFileRef;

BRMappedFileRef BRMappedFileMapFile(const char *aFilename);

void BRMappedFileUnmapFile(BRMappedFileRef aMappedFile);

void  *BRMappedFileGetData(BRMappedFileRef aMappedFile);
size_t BRMappedFileGetSize(BRMappedFileRef aMappedFile);

#endif
