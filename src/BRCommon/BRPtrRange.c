#include "BRPtrRange.h"

#include "BREndian.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct BRPtrRange
{
	uint8_t *orig_p;
	uint8_t *p;
	uint8_t *end;
};

void BRPtrRange_dummy()
{
	static char FloatIs4BytesTest[sizeof(float) == 4 ? 1 : -1];
	(void)FloatIs4BytesTest;
}

BRPtrRangeRef BRPtrRangeCreate(uint8_t *begin, uint8_t *end)
{
	assert(begin <= end);

	BRPtrRangeRef r = calloc(1, sizeof(struct BRPtrRange));
	if (!r)
		return 0;

	r->orig_p = r->p = begin;
	r->end = end;

	return r;
}

BRPtrRangeRef BRPtrRangeCreateCopy(BRPtrRangeRef aPtrRange)
{
	BRPtrRangeRef r = calloc(1, sizeof(struct BRPtrRange));
	if (!r)
		return 0;

	r->orig_p = aPtrRange->orig_p;
	r->p      = aPtrRange->p;
	r->end    = aPtrRange->end;

	return r;
}

uint8_t *BRPtrRangeGetBegin(BRPtrRangeRef r)
{
	return r->p;
}

uint8_t *BRPtrRangeGetEnd(BRPtrRangeRef r)
{
	return r->end;
}

void BRPtrRangeSetEnd(BRPtrRangeRef r, uint8_t *end)
{
	assert(r->p <= r->end);
	r->end = end;
	assert(r->p <= r->end);
}

size_t BRPtrRangeGetDistance(BRPtrRangeRef r)
{
	return r->end - r->p;
}

void BRPtrRangeSetDistance(BRPtrRangeRef r, size_t aDistance)
{
	r->end = r->p + aDistance;
}

size_t BRPtrRangeGetAdvancement(BRPtrRangeRef r)
{
	return r->p - r->orig_p;
}

void BRPtrRangeAdvance(BRPtrRangeRef r, int aAdvancement)
{
	//assert(r->end - r->p >= 0);
	//assert((size_t)(r->end - r->p) >= aAdvancement);
	r->p += aAdvancement;
}

uint8_t BRPtrRangeReadByte(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 1);
	return *(r->p);
}

uint16_t BRPtrRangeReadLE16(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 2);

	uint16_t a;
	memcpy(&a, r->p, 2);
	return letoh16(a);
}

uint16_t BRPtrRangeReadBE16(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 2);

	uint16_t a;
	memcpy(&a, r->p, 2);
	return betoh16(a);
}

uint32_t BRPtrRangeReadLE32(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 4);

	uint32_t a;
	memcpy(&a, r->p, 4);
	return letoh32(a);
}

uint32_t BRPtrRangeReadBE32(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 4);

	uint32_t a;
	memcpy(&a, r->p, 4);
	return betoh32(a);
}

float BRPtrRangeReadLEFloat(BRPtrRangeRef r)
{
	uint32_t i = BRPtrRangeReadLE32(r);
	float f;
	memcpy(&f, &i, 4);
	return f;
}

uint8_t BRPtrRangeReadByteAndAdvance(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 1);
	return *(r->p++);
}

uint16_t BRPtrRangeReadLE16AndAdvance(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 2);

	uint16_t a;
	memcpy(&a, r->p, 2);
	r->p += 2;
	return letoh16(a);
}

uint16_t BRPtrRangeReadBE16AndAdvance(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 2);

	uint16_t a;
	memcpy(&a, r->p, 2);
	r->p += 2;
	return betoh16(a);
}

uint32_t BRPtrRangeReadLE32AndAdvance(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 4);

	uint32_t a;
	memcpy(&a, r->p, 4);
	r->p += 4;
	return letoh32(a);
}

uint32_t BRPtrRangeReadBE32AndAdvance(BRPtrRangeRef r)
{
	assert(r->end - r->p >= 4);

	uint32_t a;
	memcpy(&a, r->p, 4);
	r->p += 4;
	return betoh32(a);
}

float BRPtrRangeReadLEFloatAndAdvance(BRPtrRangeRef r)
{
	uint32_t i = BRPtrRangeReadLE32AndAdvance(r);
	float f;
	memcpy(&f, &i, 4);
	return f;
}
