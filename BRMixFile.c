#include "BRMixFile.h"

#include "BRMappedFile.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

struct BRMixFile
{
	char *_filename;
	BRMappedFileRef _mappedFile;

	uint16_t _entryCount;
	uint32_t _size;

	struct _BRMixFileEntry *_entries;
};

struct _BRMixFileEntry
{
	uint32_t id;
	uint32_t offset;
	uint32_t length;
};


BRMixFileRef BRMixFileOpen(const char *aFilename)
{
	BRMixFileRef mixFile = calloc(1, sizeof(struct BRMixFile));

	if (!mixFile)
		return 0;

	mixFile->_filename = strdup(aFilename);

	mixFile->_entries = 0;

	mixFile->_mappedFile = BRMappedFileMapFile(aFilename);

	if (!mixFile->_mappedFile)
		goto cleanup;

	byte *p   = BRMappedFileGetData(mixFile->_mappedFile);
	byte *end = p + BRMappedFileGetSize(mixFile->_mappedFile);

	BRPtrRangeRef r = BRPtrRangeCreate(p, end);

	mixFile->_entryCount = BRPtrRangeReadLE16AndAdvance(r);
	mixFile->_size       = BRPtrRangeReadLE32AndAdvance(r);

	//printf("entryCount: %u\n", mixFile->_entryCount);
	//printf("size:       %u\n", mixFile->_size);

	// TODO: Verify size against map size
	if (mixFile->_size > BRMappedFileGetSize(mixFile->_mappedFile))
		goto cleanup;

	mixFile->_entries = calloc(mixFile->_entryCount, sizeof(struct _BRMixFileEntry));
	if (!mixFile->_entries)
		goto cleanup;

	size_t i;
	for (i = 0; i != mixFile->_entryCount; ++i)
	{
		mixFile->_entries[i].id     = BRPtrRangeReadLE32AndAdvance(r);
		mixFile->_entries[i].offset = BRPtrRangeReadLE32AndAdvance(r);
		mixFile->_entries[i].length = BRPtrRangeReadLE32AndAdvance(r);

		/*
		printf("Entry %3zu: %08x %08x %08x\n", i,
			mixFile->_entries[i].id,
			mixFile->_entries[i].offset,
			mixFile->_entries[i].length
		);
		*/
	}

	return mixFile;

cleanup:
	BRMixFileClose(mixFile);
	return 0;
}

void BRMixFileClose(BRMixFileRef aMixFile)
{
	if (!aMixFile)
		return;

	BRMappedFileUnmapFile(aMixFile->_mappedFile);
	free(aMixFile->_entries);
	free(aMixFile);
}

const char *BRMixFileGetName(BRMixFileRef aMixFile)
{
	return aMixFile->_filename;
}

uint16_t BRMixFileGetResourceCount(BRMixFileRef aMixFile)
{
	if (!aMixFile)
		return 0;

	return aMixFile->_entryCount;
}

void *_BRMixFileGetResourceDataByIndex(BRMixFileRef aMixFile, uint16_t aIndex)
{
	if (!aMixFile || aIndex >= aMixFile->_entryCount)
		return 0;

	return (uint8_t*)BRMappedFileGetData(aMixFile->_mappedFile) +
	       aMixFile->_entries[aIndex].offset +
	       6 + 12 * aMixFile->_entryCount;
}

size_t _BRMixFileGetResourceSizeByIndex(BRMixFileRef aMixFile, uint16_t aIndex)
{
	if (!aMixFile || aIndex >= aMixFile->_entryCount)
		return 0;

	return aMixFile->_entries[aIndex].length;
}

BRPtrRangeRef BRMixFileGetResourceRangeByIndex(BRMixFileRef aMixFile, uint16_t aIndex)
{
	if (!aMixFile || aIndex >= aMixFile->_entryCount)
		return 0;

	//printf("ID: 0x%08x\n", aMixFile->_entries[aIndex].id);

	uint8_t *p  = _BRMixFileGetResourceDataByIndex(aMixFile, aIndex);
	size_t size = _BRMixFileGetResourceSizeByIndex(aMixFile, aIndex);

	return BRPtrRangeCreate(p, p + size);
}

#define ROL(n) ((n << 1) | ((n >> 31) & 1))

uint32_t _BRMixFileGetIdForName(const char *aResourceName)
{
	char buffer[12] = { 0 };
	int i;

	for (i = 0; aResourceName[i] && i < 12; ++i)
		buffer[i] = toupper(aResourceName[i]);

	uint32_t id = 0;
	for (i = 0; i < 12 && buffer[i]; i += 4)
	{
		uint32_t t = buffer[i + 3] << 24
		           | buffer[i + 2] << 16
		           | buffer[i + 1] <<  8
		           | buffer[i + 0];

		id = ROL(id) + t;
	}

	//printf("_BRMixFileGetIdForName(\"%s\") = %08x\n", aResourceName, id);

	return id;
}

#undef ROL

int32_t _BRMixFileGetIndexForId(BRMixFileRef aMixFile, uint32_t aId)
{
	static BRMixFileRef cachedMixFile;
	static uint32_t     cachedId;
	static int32_t      cachedIndex;

	if (aMixFile == cachedMixFile && cachedId == aId)
		return cachedIndex;

	int32_t i;
	for (i = 0; i != aMixFile->_entryCount; ++i)
		if (aMixFile->_entries[i].id == aId)
		{
			cachedMixFile = aMixFile;
			cachedId      = aId;
			cachedIndex   = i;
			return i;
		}

	return -1;
}

BRPtrRangeRef BRMixFileGetResourceRangeByName(BRMixFileRef aMixFile, const char *aResourceName)
{
	uint32_t id = _BRMixFileGetIdForName(aResourceName);

	int32_t index = _BRMixFileGetIndexForId(aMixFile, id);
	if (index == -1)
		return 0;

	return BRMixFileGetResourceRangeByIndex(aMixFile, index);
}
