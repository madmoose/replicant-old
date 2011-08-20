#include "BRData.h"

#include "BRRetain.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>

struct BRData
{
	BRRetainable _;
	size_t  size;
	uint8_t bytes[];
};

BRDataRef BRDataCreate(size_t size)
{
	BRDataRef data = calloc(1, sizeof(struct BRData) + size);
	if (!data)
		return 0;

	data->size = size;

	return data;
}

BRDataRef BRDataCreateWithBytes(size_t size, uint8_t *bytes)
{
	assert(bytes);

	BRDataRef data = calloc(1, sizeof(struct BRData) + size);
	if (!data)
		return 0;

	data->size = size;
	memcpy(data->bytes, bytes, size);

	return data;
}

BRDataRef BRDataCreateWithData(BRDataRef aData)
{
	BRDataRef data = calloc(1, sizeof(struct BRData) + aData->size);
	if (!data)
		return 0;

	data->size = aData->size;
	memcpy(data->bytes, aData->bytes, aData->size);

	return data;
}

uint8_t *BRDataGetBytes(BRDataRef aData)
{
	assert(aData);
	assert(aData->bytes);
	return aData->bytes;
}

size_t BRDataGetSize(BRDataRef aData)
{
	assert(aData);
	return aData->size;
}
