#include "BRVQAReader.h"

#include "BRCommon.h"
#include "BRUtils.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <execinfo.h>
#endif

#define kAESC 0x41455343
#define kCBFZ 0x4342465A
#define kCIND 0x43494E44
#define kCINF 0x43494E46
#define kCINH 0x43494E48
#define kCLIP 0x434C4950
#define kFINF 0x46494E46
#define kFORM 0x464f524d
#define kLIND 0x4C494E44
#define kLINF 0x4C494E46
#define kLINH 0x4C494E48
#define kLITE 0x4C495445
#define kLNID 0x4C4E4944
#define kLNIH 0x4C4E4948
#define kLNIN 0x4C4E494E
#define kLNIO 0x4C4E494F
#define kMFCD 0x4D464344
#define kMFCH 0x4D464348
#define kMFCI 0x4D464349
#define kMFCT 0x4D464354
#define kMSCH 0x4D534348
#define kMSCI 0x4D534349
#define kMSCT 0x4D534354
#define kSN2J 0x534e324a
#define kSND2 0x534e4432
#define kVIEW 0x56494557
#define kVPTR 0x56505452
#define kVQFL 0x5651464C
#define kVQFR 0x56514652
#define kVQHD 0x56514844
#define kWVQA 0x57565141
#define kZBUF 0x5A425546

void dump_stacktrace()
{
#ifndef _WIN32
	void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);

	for (i = 1; i < frames; ++i)
		printf("%s\n", strs[i]);

	free(strs);
#endif
}

#define cleanup_if(x) do { if (x) { printf("%s %s:%d:\tcleanup_if failed: " #x "\n", __FILE__, __func__, __LINE__); goto cleanup; } } while (0)

#define cleanup_if_not(x) do { if (!(x)) { printf("%s %s:%d:\tcleanup_if_not failed: " #x "\n", __FILE__, __func__, __LINE__); goto cleanup; } } while (0)


uint32_t readTag(BRPtrRangeRef r)
{
	if (BRPtrRangeGetAdvancement(r) & 1)
	{
		cleanup_if(BRPtrRangeGetDistance(r) < 1);

		BRPtrRangeReadByteAndAdvance(r);
	}

	cleanup_if(BRPtrRangeGetDistance(r) < 4);

	return BRPtrRangeReadBE32AndAdvance(r);

cleanup:
	return 0;
}

const char *str_tag(uint32_t tag)
{
	static char s[5];

	sprintf(s, "%c%c%c%c",
		(tag >> 24) & 0xff,
		(tag >> 16) & 0xff,
		(tag >>  8) & 0xff,
		(tag >>  0) & 0xff);

	return s;
}

#define LIMIT_SIZE(r, s) \
	uint8_t *_oldEnd = BRPtrRangeGetEnd(r); \
	BRPtrRangeSetDistance(r, s)

#define UNDO_LIMIT_SIZE(r) \
	BRPtrRangeSetEnd(r, _oldEnd)

#define UNUSED(x) (void)x

struct BRVQAReader
{
	BRPtrRangeRef vqaRange;

	struct {
		uint16_t version;
		uint16_t flags;
		uint16_t numFrames;
		uint16_t width;
		uint16_t height;
		uint8_t  blockW;
		uint8_t  blockH;
		uint8_t  frameRate;
		uint8_t  cbParts;
		uint16_t colors;
		uint16_t maxBlocks;
		uint32_t unk1;
		uint16_t unk2;
		uint16_t freq;
		uint8_t  channels;
		uint8_t  bits;
		uint32_t unk3;
		uint16_t unk4;
		uint32_t maxCBFZSize;
		uint32_t unk5;
	} _header;

	uint32_t  _codeBookSize;
	uint8_t  *_codeBook;
	uint32_t *_frame;

	uint32_t _maxAESCSize;
	uint32_t _maxLITESize;
	uint32_t _maxVIEWSize;
	uint32_t _maxZBUFSize;

	uint32_t *_frameInfo;

	struct {
		uint16_t frameBegin;
		uint16_t frameEnd;
		char    *name;
	} *_loopInfo;
	uint16_t _loopCount;
};

BOOL _BRVQAReaderReadVQHD(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	uint32_t size = BRPtrRangeGetDistance(r);
	if (size != 42)
		return NO;

	aVQAReader->_header.version      = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.flags        = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.numFrames    = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.width        = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.height       = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.blockW       = BRPtrRangeReadByteAndAdvance(r);
	aVQAReader->_header.blockH       = BRPtrRangeReadByteAndAdvance(r);
	aVQAReader->_header.frameRate    = BRPtrRangeReadByteAndAdvance(r);
	aVQAReader->_header.cbParts      = BRPtrRangeReadByteAndAdvance(r);
	aVQAReader->_header.colors       = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.maxBlocks    = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.unk1         = BRPtrRangeReadLE32AndAdvance(r);
	aVQAReader->_header.unk2         = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.freq         = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.channels     = BRPtrRangeReadByteAndAdvance(r);
	aVQAReader->_header.bits         = BRPtrRangeReadByteAndAdvance(r);
	aVQAReader->_header.unk3         = BRPtrRangeReadLE32AndAdvance(r);
	aVQAReader->_header.unk4         = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_header.maxCBFZSize  = BRPtrRangeReadLE32AndAdvance(r);
	aVQAReader->_header.unk5         = BRPtrRangeReadLE32AndAdvance(r);

	printf("Version: %04x\nFlags:   %04x\n",
		aVQAReader->_header.version,
		aVQAReader->_header.flags);

	printf("Frames:  %4d (0x%x)\n",
		aVQAReader->_header.numFrames,
		aVQAReader->_header.numFrames);

	printf("Size:    %3dx%d\n",
		aVQAReader->_header.width,
		aVQAReader->_header.height);

	return YES;
}

uint16_t BRVQAReaderGetFrameCount(BRVQAReaderRef aVQAReader)
{
	assert(aVQAReader);
	return aVQAReader->_header.numFrames;
}

BRSize BRVQAReaderGetFrameSize(BRVQAReaderRef aVQAReader)
{
	assert(aVQAReader);

	BRSize size = { aVQAReader->_header.width, aVQAReader->_header.height };

	return size;
}

#define DUMP do { hexdump(BRPtrRangeGetBegin(r)-8, size+8); } while (0)

BOOL _BRVQAReaderReadCLIP(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);

	uint32_t size = BRPtrRangeGetDistance(r);
	if (size != 8)
		return NO;

	uint32_t width =  BRPtrRangeReadLE32AndAdvance(r);
	uint32_t height = BRPtrRangeReadLE32AndAdvance(r);

	printf("CLIP: %d x %d\n", width, height);

	UNUSED(width);
	UNUSED(height);

	return YES;
}

/*
 * LINF = Loop Info
 */
BOOL _BRVQAReaderReadLINF(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	cleanup_if(BRPtrRangeGetDistance(r) < 14);

	// Loop info header
	cleanup_if_not(readTag(r) == kLINH);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == 6);

	aVQAReader->_loopCount = BRPtrRangeReadLE16AndAdvance(r);
	aVQAReader->_loopInfo = calloc(aVQAReader->_loopCount, sizeof(aVQAReader->_loopInfo[0]));
	cleanup_if_not(BRPtrRangeReadLE32AndAdvance(r) == 2);

	// Loop info data
	cleanup_if(BRPtrRangeGetDistance(r) != 8 + 4 * aVQAReader->_loopCount);
	cleanup_if_not(readTag(r) == kLIND);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == 4 * aVQAReader->_loopCount);

	unsigned int i;
	for (i = 0; i != aVQAReader->_loopCount; ++i)
	{
		aVQAReader->_loopInfo[i].frameBegin = BRPtrRangeReadLE16AndAdvance(r);
		aVQAReader->_loopInfo[i].frameEnd   = BRPtrRangeReadLE16AndAdvance(r);
	}

	return YES;
cleanup:
	return NO;
}

/*
 * LNIN = Loop Names
 */
BOOL _BRVQAReaderReadLNIN(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	BOOL rc = NO;
	uint32_t *loopNameOffsets = 0;

	uint32_t size = BRPtrRangeGetDistance(r);
	UNUSED(size);

	cleanup_if_not(readTag(r) == kLNIH);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == 10);

	uint16_t loopNamesCount = BRPtrRangeReadLE16AndAdvance(r);
	uint16_t loopUnk1       = BRPtrRangeReadLE16AndAdvance(r);
	uint16_t loopUnk2       = BRPtrRangeReadLE16AndAdvance(r);
	uint16_t loopUnk3       = BRPtrRangeReadLE16AndAdvance(r);
	uint16_t loopUnk4       = BRPtrRangeReadLE16AndAdvance(r);

	UNUSED(loopUnk1);
	UNUSED(loopUnk2);
	UNUSED(loopUnk4);

	cleanup_if_not(readTag(r) == kLNIO);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == 4 * loopNamesCount);

	loopNameOffsets = calloc(loopNamesCount, sizeof(uint32_t));

	unsigned int i;
	for (i = 0; i != loopNamesCount; ++i)
		loopNameOffsets[i] = BRPtrRangeReadLE32AndAdvance(r);

	cleanup_if_not(readTag(r) == kLNID);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == loopUnk3);

	char *namesStart = (char*)BRPtrRangeGetBegin(r);
	for (i = 0; i != loopNamesCount; ++i)
		aVQAReader->_loopInfo[i].name = namesStart + loopNameOffsets[i];

	BRPtrRangeAdvance(r, loopUnk3);

	rc = YES;
cleanup:
	free(loopNameOffsets);
	return rc;
}

/*
 * The MFCI chunk has some frame info and MFCT lists the
 * max chunk size of LITE. TBD.
 */
BOOL _BRVQAReaderReadMFCI(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	uint32_t size = BRPtrRangeGetDistance(r);
	//printf("MFCI: %d\n", size);

	//DUMP;

	BRPtrRangeAdvance(r, size);

	return YES;
}

/*
 * The MSCI chunk lists the max chunk size of VIEW, ZBUF, and AESC. TBD.
 */
BOOL _BRVQAReaderReadMSCI(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	uint32_t size = BRPtrRangeGetDistance(r);
	//printf("MSCI: %d\n", size);

	//DUMP;

	BRPtrRangeAdvance(r, size);

	return YES;
}

/*
 * CINF = Clip info?
 */
BOOL _BRVQAReaderReadCINF(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);

	//uint32_t size = BRPtrRangeGetDistance(r);

	cleanup_if_not(readTag(r) == kCINH);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == 8);

	uint16_t clipCount = BRPtrRangeReadLE16AndAdvance(r);
	uint16_t unkFrames = BRPtrRangeReadLE16AndAdvance(r);
	uint32_t unk2      = BRPtrRangeReadLE32AndAdvance(r);

	UNUSED(unkFrames);
	cleanup_if_not(unk2 == 0);

	cleanup_if_not(readTag(r) == kCIND);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) == 6 * clipCount);

	int i;
	for (i = 0; i != clipCount; ++i)
	{
		/*
		printf("Clip %d:\n", i);
		hexdump(BRPtrRangeGetBegin(r), 6);
		printf("%d ", BRPtrRangeReadLE16AndAdvance(r));
		printf("%d ", BRPtrRangeReadLE16AndAdvance(r));
		printf("%d\n", BRPtrRangeReadLE16AndAdvance(r));
		*/
		BRPtrRangeAdvance(r, 6);
	}

	//BRPtrRangeAdvance(r, 6 * clipCount);

	return YES;
cleanup:
	return NO;
}

/*
 * FINF = Frame Info.
 * Offsets of frames + some flags
 */

BOOL _BRVQAReaderReadFINF(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	uint16_t numFrames = aVQAReader->_header.numFrames;

	uint32_t size = BRPtrRangeGetDistance(r);
	cleanup_if(size != 4 * numFrames);

	aVQAReader->_frameInfo = calloc(numFrames, sizeof(uint32_t));

	unsigned int i;
	for (i = 0; i != numFrames; ++i)
		aVQAReader->_frameInfo[i] = BRPtrRangeReadLE32AndAdvance(r);

	for (i = 0; i != numFrames; ++i)
	{
		uint32_t flags = aVQAReader->_frameInfo[i] & ~0x1fffffff;
		if (flags)
			printf("Frame flags: %3d  %08x\n", i, flags);
	}

	return YES;
cleanup:
	return NO;
}

uint32_t decodeFormat80(uint8_t *inbuf, uint8_t *outbuf, uint32_t inbuflen)
{
	int version = 1;
	int count, color;
	int pos, relpos;
	int i;

	uint8_t *src = inbuf;
	uint8_t *dst = outbuf;

	if (src[0] == 0)
	{
		version = 2;
		++src;
	}

	while (src[0] != 0x80 && (src - inbuf) < inbuflen)
	{
		if (src[0] == 0xff)      // 0b11111111
		{
			count = src[1] | (src[2] << 8);
			pos   = src[3] | (src[4] << 8);
			src += 5;

			if (version == 1)
			{
				for (i = 0; i < count; i++)
					dst[i] = outbuf[i + pos];
			}
			else
			{
				for (i = 0; i < count; i++)
					dst[i] = dst[i - pos];
			}
		}
		else if (src[0] == 0xfe) // 0b11111110
		{
			count = src[1] | (src[2] << 8);
			color = src[3];
			src += 4;

			memset(dst, color, count);
		}
		else if (src[0] >= 0xc0)  // 0b11??????
		{
			count = (src[0] & 0x3f) + 3;
			pos   = src[1] | (src[2] << 8);
			src += 3;

			if (version == 1)
			{
				for (i = 0; i < count; i++)
					dst[i] = outbuf[i + pos];
			}
			else
			{
				for (i = 0; i < count; i++)
					dst[i] = dst[i - pos];
			}
		}
		else if (src[0] >= 0x80)  // 0b10??????
		{
			count = src[0] & 0x3f;
			++src;
			memcpy(dst, src, count);
			src += count;
		}
		else                     // 0b0???????
		{
			count  = ((src[0] & 0x70) >> 4) + 3;
			relpos = ((src[0] & 0x0f) << 8) | src[1];

			src += 2;

			for (i = 0; i < count; i++)
				dst[i] = dst[i - relpos];
		}

		dst += count;
	}

	return dst - outbuf;
}

uint32_t decodeFormat80OutputSize(uint8_t *inbuf, uint32_t inbuflen)
{
	int count;
	uint8_t *src     = inbuf;
	uint32_t outsize = 0;

	if (src[0] == 0)
		++src;

	while (src[0] != 0x80 && (src - inbuf) < inbuflen)
	{
		if (src[0] == 0xff)      // 0b11111111
		{
			count = src[1] | (src[2] << 8);
			src += 5;
		}
		else if (src[0] == 0xfe) // 0b11111110
		{
			count = src[1] | (src[2] << 8);
			src += 4;
		}
		else if (src[0] >= 0xc0)  // 0b11??????
		{
			count = (src[0] & 0x3f) + 3;
			src += 3;
		}
		else if (src[0] & 0x80)  // 0b10??????
		{
			count = src[0] & 0x3f;
			src += count + 1;
		}
		else                     // 0b0???????
		{
			count  = ((src[0] & 0x70) >> 4) + 3;
			src += 2;
		}

		outsize += count;
	}

	return outsize;
}

BOOL _BRVQAReaderReadCBFZ(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	uint32_t size = BRPtrRangeGetDistance(r);

	uint32_t out_size = decodeFormat80OutputSize(BRPtrRangeGetBegin(r), size);

	//printf("\tCBFZ: size: %6d\toutsize: %6d\n", size, out_size);
	if (out_size > aVQAReader->_codeBookSize)
	{
		// VQA2.MIX resource 11 triggers this with 16001 > 16000.
		// Should probably ignore overflow.
		printf("ERROR CBFZ: %d > %d\n", out_size, aVQAReader->_codeBookSize);
		return NO;
	}

	decodeFormat80(BRPtrRangeGetBegin(r), aVQAReader->_codeBook, size);

	BRPtrRangeAdvance(r, size);

	return YES;
}

// TODO: Alpha parameter should be an enum.

void _BRVQAReaderReadVPTRWriteBlock(BRVQAReaderRef aVQAReader, unsigned int dstBlock, unsigned int srcBlock, int count, BOOL alpha)
{
	uint16_t frameWidth  = aVQAReader->_header.width;
	uint16_t frameHeight = aVQAReader->_header.height;
	uint16_t blockWidth  = aVQAReader->_header.blockW;
	uint16_t blockHeight = aVQAReader->_header.blockH;

	const uint8_t *const blockSrc = &aVQAReader->_codeBook[2 * srcBlock * blockWidth * blockHeight];
	uint8_t blocksPerLine = frameWidth / blockWidth;

	do
	{
		uint32_t frameX = dstBlock % blocksPerLine * blockWidth;
		uint32_t frameY = dstBlock / blocksPerLine * blockHeight;

		const uint8_t *__restrict__ src = blockSrc;
		uint32_t      *__restrict__ dst = &aVQAReader->_frame[frameX + frameY * frameWidth];

		unsigned int blockY;
		for (blockY = 0; blockY != blockHeight; ++blockY)
		{
			unsigned int blockX;
			for (blockX = 0; blockX != blockWidth; ++blockX)
			{
				assert(dst - aVQAReader->_frame < frameWidth * frameHeight);

				uint16_t rgb555 = src[0] | (src[1] << 8);
				src += 2;

				uint8_t r, g, b, a;
				r = g = b = a = 0;

				r = (rgb555 >> 10) & 0x1f;
				g = (rgb555 >>  5) & 0x1f;
				b = (rgb555 >>  0) & 0x1f;
				a = 255;

				r = (r << 3) | (r >> 2);
				g = (g << 3) | (g >> 2);
				b = (b << 3) | (b >> 2);

				uint32_t rgba = (a << 24) | (b << 16) | (g << 8) | r;
				if (alpha && rgba == 0xff000000)
					*dst++ = 0xffffff00;
				else
					*dst++ = rgba;
			}
			dst += frameWidth - blockWidth;
		}

		++dstBlock;
	}
	while (--count);
}

BOOL _BRVQAReaderReadVPTR(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	uint16_t frameWidth  = aVQAReader->_header.width;
	uint16_t frameHeight = aVQAReader->_header.height;
	uint16_t blockWidth  = aVQAReader->_header.blockW;
	uint16_t blockHeight = aVQAReader->_header.blockH;

	uint8_t *data = BRPtrRangeGetBegin(r);
	uint32_t size = BRPtrRangeGetDistance(r);

	uint16_t command, count, srcBlock;
	unsigned int dstBlock = 0;
	unsigned int dstBlocks = (frameWidth / blockWidth) * (frameHeight / blockHeight);

	uint8_t *src = data;
	uint8_t *end = data + size;

	BRPtrRangeAdvance(r, size);

	while (DISTANCE(src, end) >= 2)
	{
		command = src[0] | (src[1] << 8);
		uint8_t prefix = command >> 13;
		src += 2;

		switch (prefix)
		{
		case 0:
			count = command & 0x1fff;
			dstBlock += count;
			break;
		case 1:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			cleanup_if_not(srcBlock < aVQAReader->_codeBookSize);
			cleanup_if_not(dstBlock + count <= dstBlocks);

			_BRVQAReaderReadVPTRWriteBlock(aVQAReader, dstBlock, srcBlock, count, NO);
			dstBlock += count;
			break;
		case 2:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			cleanup_if_not(srcBlock < aVQAReader->_codeBookSize);
			cleanup_if_not(dstBlock + count + 1 <= dstBlocks);

			_BRVQAReaderReadVPTRWriteBlock(aVQAReader, dstBlock++, srcBlock, 1, NO);

			int i;
			for (i = 0; i < count; ++i)
			{
				srcBlock = *src++;
				cleanup_if_not(srcBlock < aVQAReader->_codeBookSize);
				_BRVQAReaderReadVPTRWriteBlock(aVQAReader, dstBlock++, srcBlock, 1, NO);
			}
			break;
		case 3:
		case 4:
			count = 1;
			srcBlock = command & 0x1fff;

			cleanup_if_not(srcBlock < aVQAReader->_codeBookSize);
			cleanup_if_not(dstBlock + count <= dstBlocks);

			_BRVQAReaderReadVPTRWriteBlock(aVQAReader, dstBlock++, srcBlock, count, prefix == 4);
			break;
		case 5:
		case 6:
			count = *src++;
			srcBlock = command & 0x1fff;

			cleanup_if_not(srcBlock < aVQAReader->_codeBookSize);
			cleanup_if_not(dstBlock < dstBlocks);

			_BRVQAReaderReadVPTRWriteBlock(aVQAReader, dstBlock, srcBlock, count, prefix == 6);
			dstBlock += count;
			break;
		default:
			printf("Undefined case %d\n", command >> 13);
		}
	}
	return YES;

cleanup:
	return NO;
}

BOOL _BRVQAReaderReadVQFR(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	//uint32_t size = BRPtrRangeGetDistance(r);
	//printf("VQFR: %d\n", size);

	while (BRPtrRangeGetDistance(r) >= 8)
	{
		uint32_t tag = readTag(r);
		printf("\tTag: %s\n", str_tag(tag));
		uint32_t size = BRPtrRangeReadBE32AndAdvance(r);

		cleanup_if(size > BRPtrRangeGetDistance(r));
		LIMIT_SIZE(r, size);

		BOOL rc = NO;
		switch (tag)
		{
			case kCBFZ:
				rc = _BRVQAReaderReadCBFZ(aVQAReader, r);
				break;
			case kVPTR:
				rc = _BRVQAReaderReadVPTR(aVQAReader, r);
				break;
			default:
				puts("Unknown tag:");
				hexdump(BRPtrRangeGetBegin(r) - 8, 16);
				printf("Advancing %d\n", size);
				BRPtrRangeAdvance(r, size);
				break;
		}

		UNDO_LIMIT_SIZE(r);
		cleanup_if_not(rc);
	}

	return YES;
cleanup:
	return NO;
}

BOOL _BRVQAReaderReadVQFL(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	//uint32_t size = BRPtrRangeGetDistance(r);
	//printf("VQFL: %d\n", size);

	while (BRPtrRangeGetDistance(r) >= 8)
	{
		uint32_t tag = readTag(r);
		printf("\tTag: %s\n", str_tag(tag));

		uint32_t size = BRPtrRangeReadBE32AndAdvance(r);

		cleanup_if(size > BRPtrRangeGetDistance(r));
		LIMIT_SIZE(r, size);

		BOOL rc = NO;
		switch (tag)
		{
			case kCBFZ:
				rc = _BRVQAReaderReadCBFZ(aVQAReader, r);
				break;
			default:
				puts("Unknown tag:");
				hexdump(BRPtrRangeGetBegin(r) - 8, 16);
				printf("Advancing %d\n", size);
				BRPtrRangeAdvance(r, size);
				break;
		}

		UNDO_LIMIT_SIZE(r);
		cleanup_if_not(rc);
	}

	return YES;
cleanup:
	return NO;
}

BOOL _BRVQAReaderReadVIEW(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	uint32_t size = BRPtrRangeGetDistance(r);
	//printf("VIEW: %d\n", size);
	if (size != 56)
		return NO;

	//DUMP;

	BRPtrRangeAdvance(r, size);

	return YES;
}

BOOL _BRVQAReaderReadZBUF(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	uint32_t size = BRPtrRangeGetDistance(r);
	//printf("ZBUF: %d\n", size);

	//DUMP;

	BRPtrRangeAdvance(r, size);

	return YES;
}

BOOL _BRVQAReaderReadAESC(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	uint32_t size = BRPtrRangeGetDistance(r);
	//printf("AESC: %d\n", size);

	//DUMP;

	BRPtrRangeAdvance(r, size);

	return YES;
}

BOOL _BRVQAReaderReadLITE(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	UNUSED(aVQAReader);
	uint32_t size = BRPtrRangeGetDistance(r);
	//printf("LITE: %d\n", size);

	//DUMP;

	BRPtrRangeAdvance(r, size);

	return YES;
}

BOOL BRVQAReaderReadFrame(BRVQAReaderRef aVQAReader, unsigned int aFrameNumber)
{
	cleanup_if_not(aVQAReader);

	BRPtrRangeRef r = BRPtrRangeCreateCopy(aVQAReader->vqaRange);
	cleanup_if_not(r);

	BRPtrRangeAdvance(r, 2 * (aVQAReader->_frameInfo[aFrameNumber] & 0x1fffffff));

	uint32_t tag;
	do {
		tag = readTag(r);
		printf("Tag: %s\n", str_tag(tag));

		cleanup_if(BRPtrRangeGetDistance(r) < 4);
		uint32_t size = BRPtrRangeReadBE32AndAdvance(r);

		cleanup_if(BRPtrRangeGetDistance(r) < size);
		LIMIT_SIZE(r, size);

		BOOL rc = NO;
		switch (tag)
		{
			case kVQFL:
				rc = _BRVQAReaderReadVQFL(aVQAReader, r);
				break;
			case kVQFR:
				rc = _BRVQAReaderReadVQFR(aVQAReader, r);
				break;
			case kVIEW:
				rc = _BRVQAReaderReadVIEW(aVQAReader, r);
				break;
			case kZBUF:
				rc = _BRVQAReaderReadZBUF(aVQAReader, r);
				break;
			case kAESC:
				rc = _BRVQAReaderReadAESC(aVQAReader, r);
				break;
			case kLITE:
				rc = _BRVQAReaderReadLITE(aVQAReader, r);
				break;
			case kSN2J:
			case kSND2:
				// Skip for now
				BRPtrRangeAdvance(r, size);
				rc = YES;
				break;
			default:
				puts("Unknown or unexpected tag:");
				hexdump(BRPtrRangeGetBegin(r) - 8, 16);
				BRPtrRangeAdvance(r, size);
		}

		UNDO_LIMIT_SIZE(r);
		cleanup_if_not(rc);
	} while (tag != kVQFR);

	return YES;
cleanup:
	return NO;
}

uint8_t *BRVQAReaderGetFrame(BRVQAReaderRef aVQAReader)
{
	return (uint8_t*)aVQAReader->_frame;
}


BRVQAReaderRef BRVQAReaderOpen(BRPtrRangeRef r)
{
	BRVQAReaderRef VQAReader = calloc(1, sizeof(struct BRVQAReader));
	cleanup_if_not(VQAReader);

	VQAReader->vqaRange = BRPtrRangeCreateCopy(r);

	uint32_t size;
	uint32_t tag;
	BOOL rc;

	//cleanup_if_not(BRPtrRangeGetDistance(r) >= ...);

	tag = readTag(r);
	cleanup_if_not(tag == kFORM);

	size = BRPtrRangeReadBE32AndAdvance(r);
	//cleanup_if_not(size > ...);

	uint32_t sig = BRPtrRangeReadBE32AndAdvance(r);
	cleanup_if_not(sig == kWVQA);

	tag = readTag(r);
	cleanup_if_not(tag == kVQHD);
	size = BRPtrRangeReadBE32AndAdvance(r);
	LIMIT_SIZE(r, size);
	rc = _BRVQAReaderReadVQHD(VQAReader, r);
	UNDO_LIMIT_SIZE(r);
	cleanup_if_not(rc);

	VQAReader->_codeBookSize = 2 * VQAReader->_header.maxBlocks *
	                               VQAReader->_header.blockW *
	                               VQAReader->_header.blockH;
	VQAReader->_codeBook  = malloc(VQAReader->_codeBookSize);

	VQAReader->_frame     = malloc(4 * VQAReader->_header.width *
	                                   VQAReader->_header.height);

	cleanup_if_not(VQAReader->_codeBook);
	cleanup_if_not(VQAReader->_frame);

	do {
		tag = readTag(r);
		printf("Tag: %s\n", str_tag(tag));

		cleanup_if(BRPtrRangeGetDistance(r) < 4);
		uint32_t size = BRPtrRangeReadBE32AndAdvance(r);

		cleanup_if(BRPtrRangeGetDistance(r) < size);
		LIMIT_SIZE(r, size);

		BOOL rc = NO;
		switch (tag)
		{
			case kCLIP:
				rc = _BRVQAReaderReadCLIP(VQAReader, r);
				break;
			case kLINF:
				rc = _BRVQAReaderReadLINF(VQAReader, r);
				break;
			case kLNIN:
				rc = _BRVQAReaderReadLNIN(VQAReader, r);
				break;
			case kMFCI:
				rc = _BRVQAReaderReadMFCI(VQAReader, r);
				break;
			case kMSCI:
				rc = _BRVQAReaderReadMSCI(VQAReader, r);
				break;
			case kCINF:
				rc = _BRVQAReaderReadCINF(VQAReader, r);
				break;
			case kFINF:
				rc = _BRVQAReaderReadFINF(VQAReader, r);
				break;
			default:
				puts("Unknown or unexpected tag:");
				hexdump(BRPtrRangeGetBegin(r) - 8, 16);
				BRPtrRangeAdvance(r, size);
		}

		UNDO_LIMIT_SIZE(r);
		cleanup_if_not(rc);
	} while (tag != kFINF);

	int i;
	for (i = 0; i != VQAReader->_loopCount; ++i)
	{
		printf("\tLoop %2d: %04x - %04x  %s\n", i,
			VQAReader->_loopInfo[i].frameBegin,
			VQAReader->_loopInfo[i].frameEnd,
			VQAReader->_loopInfo[i].name ? VQAReader->_loopInfo[i].name : "");
	}

	return VQAReader;

cleanup:
	BRVQAReaderClose(VQAReader);
	return 0;
}

void BRVQAReaderClose(BRVQAReaderRef aVQAReader)
{
	if (!aVQAReader)
		return;

	free(aVQAReader->_loopInfo);
	free(aVQAReader->_frameInfo);
	free(aVQAReader->_frame);
	free(aVQAReader->_codeBook);
	free(aVQAReader);
}
