#include "BRVQAReader.h"

#include "BRCommon.h"
#include "BRRetain.h"
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

	uint32_t  _frameStride;
	uint8_t  *_frame;

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

	BRDataRef audioFrame;
	struct {
		int step_index;
		int predictor;
	} adpcm;

	BRSize _clipSize;
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

	/*
	printf("Version: %04x\nFlags:   %04x\n",
		aVQAReader->_header.version,
		aVQAReader->_header.flags);

	printf("Frames:  %4d (0x%x)\n",
		aVQAReader->_header.numFrames,
		aVQAReader->_header.numFrames);

	printf("Size:    %3dx%d\n",
		aVQAReader->_header.width,
		aVQAReader->_header.height);
	*/

	return YES;
}

uint16_t BRVQAReaderGetFrameCount(BRVQAReaderRef aVQAReader)
{
	assert(aVQAReader);
	return aVQAReader->_header.numFrames;
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

	aVQAReader->_clipSize = BRSizeMake(width, height);

	/*
	printf("CLIP: %d x %d\n", aVQAReader->_clipSize.width,
	                          aVQAReader->_clipSize.height);
	*/

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
	aVQAReader->_loopInfo = calloc(aVQAReader->_loopCount,
	                               sizeof(aVQAReader->_loopInfo[0]));
	cleanup_if_not(BRPtrRangeReadLE32AndAdvance(r) == 2);

	// Loop info data
	cleanup_if(BRPtrRangeGetDistance(r) != 8 + 4 * aVQAReader->_loopCount);
	cleanup_if_not(readTag(r) == kLIND);
	cleanup_if_not(BRPtrRangeReadBE32AndAdvance(r) ==
	               4 * aVQAReader->_loopCount);

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
		uint16_t unk3 = BRPtrRangeReadLE16AndAdvance(r);
		uint16_t unk4 = BRPtrRangeReadLE32AndAdvance(r);
		UNUSED(unk3);
		UNUSED(unk4);

		/*
		printf("\nClip %d: ", i);
		printf("%5x ", unk3);
		printf("%8d ", unk4);
		putchar('\n');
		hexdump(BRPtrRangeGetBegin(r) - 6, 6);
		*/
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
		//printf("frameInfo[%3d] = %08x\n", i, aVQAReader->_frameInfo[i]);
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

void _BRVQAReaderReadVPTRWriteBlock(BRVQAReaderRef aVQAReader,
		unsigned int dstBlock, unsigned int srcBlock, int count, BOOL alpha)
{
	uint16_t frameWidth  = aVQAReader->_header.width;
	uint32_t frameStride = aVQAReader->_frameStride;
	uint16_t blockWidth  = aVQAReader->_header.blockW;
	uint16_t blockHeight = aVQAReader->_header.blockH;
	int bpp = 4;

	const uint8_t *const blockSrc =
		&aVQAReader->_codeBook[2 * srcBlock * blockWidth * blockHeight];

	uint8_t blocksPerLine = frameWidth / blockWidth;

	do
	{
		uint32_t frameX = dstBlock % blocksPerLine * blockWidth;
		uint32_t frameY = dstBlock / blocksPerLine * blockHeight;

		uint32_t dstOffset = bpp * (frameX + frameY * frameStride);

		const uint8_t *__restrict__ src = blockSrc;
		uint8_t       *__restrict__ dst = &aVQAReader->_frame[dstOffset];

		unsigned int blockY;
		for (blockY = 0; blockY != blockHeight; ++blockY)
		{
			unsigned int blockX;
			for (blockX = 0; blockX != blockWidth; ++blockX)
			{
				uint16_t rgb555 = src[0] | (src[1] << 8);
				src += 2;

				if (alpha && (rgb555 & 0x8000))
				{
					dst += 4;
					continue;
				}

				uint8_t r, g, b, a;

				r = (rgb555 >> 10) & 0x1f;
				g = (rgb555 >>  5) & 0x1f;
				b = (rgb555 >>  0) & 0x1f;
				a = 255;

				r = (r << 3) | (r >> 2);
				g = (g << 3) | (g >> 2);
				b = (b << 3) | (b >> 2);

				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = a;

			}
			dst += bpp * (frameStride - blockWidth);
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
	unsigned int dstBlocks = (frameWidth / blockWidth) *
	                         (frameHeight / blockHeight);

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
		//printf("\tTag: %s\n", str_tag(tag));
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
		//printf("\tTag: %s\n", str_tag(tag));

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

static const
int16_t ima_index_table[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

static const
uint16_t ima_step_table[712] =
{
 0x0000,0x0001,0x0003,0x0004,0x0007,0x0008,0x000a,0x000b,
 0x0001,0x0003,0x0005,0x0007,0x0009,0x000b,0x000d,0x000f,
 0x0001,0x0003,0x0005,0x0007,0x000a,0x000c,0x000e,0x0010,
 0x0001,0x0003,0x0006,0x0008,0x000b,0x000d,0x0010,0x0012,
 0x0001,0x0003,0x0006,0x0008,0x000c,0x000e,0x0011,0x0013,
 0x0001,0x0004,0x0007,0x000a,0x000d,0x0010,0x0013,0x0016,
 0x0001,0x0004,0x0007,0x000a,0x000e,0x0011,0x0014,0x0017,
 0x0001,0x0004,0x0008,0x000b,0x000f,0x0012,0x0016,0x0019,
 0x0002,0x0006,0x000a,0x000e,0x0012,0x0016,0x001a,0x001e,
 0x0002,0x0006,0x000a,0x000e,0x0013,0x0017,0x001b,0x001f,
 0x0002,0x0006,0x000b,0x000f,0x0015,0x0019,0x001e,0x0022,
 0x0002,0x0007,0x000c,0x0011,0x0017,0x001c,0x0021,0x0026,
 0x0002,0x0007,0x000d,0x0012,0x0019,0x001e,0x0024,0x0029,
 0x0003,0x0009,0x000f,0x0015,0x001c,0x0022,0x0028,0x002e,
 0x0003,0x000a,0x0011,0x0018,0x001f,0x0026,0x002d,0x0034,
 0x0003,0x000a,0x0012,0x0019,0x0022,0x0029,0x0031,0x0038,
 0x0004,0x000c,0x0015,0x001d,0x0026,0x002e,0x0037,0x003f,
 0x0004,0x000d,0x0016,0x001f,0x0029,0x0032,0x003b,0x0044,
 0x0005,0x000f,0x0019,0x0023,0x002e,0x0038,0x0042,0x004c,
 0x0005,0x0010,0x001b,0x0026,0x0032,0x003d,0x0048,0x0053,
 0x0006,0x0012,0x001f,0x002b,0x0038,0x0044,0x0051,0x005d,
 0x0006,0x0013,0x0021,0x002e,0x003d,0x004a,0x0058,0x0065,
 0x0007,0x0016,0x0025,0x0034,0x0043,0x0052,0x0061,0x0070,
 0x0008,0x0018,0x0029,0x0039,0x004a,0x005a,0x006b,0x007b,
 0x0009,0x001b,0x002d,0x003f,0x0052,0x0064,0x0076,0x0088,
 0x000a,0x001e,0x0032,0x0046,0x005a,0x006e,0x0082,0x0096,
 0x000b,0x0021,0x0037,0x004d,0x0063,0x0079,0x008f,0x00a5,
 0x000c,0x0024,0x003c,0x0054,0x006d,0x0085,0x009d,0x00b5,
 0x000d,0x0027,0x0042,0x005c,0x0078,0x0092,0x00ad,0x00c7,
 0x000e,0x002b,0x0049,0x0066,0x0084,0x00a1,0x00bf,0x00dc,
 0x0010,0x0030,0x0051,0x0071,0x0092,0x00b2,0x00d3,0x00f3,
 0x0011,0x0034,0x0058,0x007b,0x00a0,0x00c3,0x00e7,0x010a,
 0x0013,0x003a,0x0061,0x0088,0x00b0,0x00d7,0x00fe,0x0125,
 0x0015,0x0040,0x006b,0x0096,0x00c2,0x00ed,0x0118,0x0143,
 0x0017,0x0046,0x0076,0x00a5,0x00d5,0x0104,0x0134,0x0163,
 0x001a,0x004e,0x0082,0x00b6,0x00eb,0x011f,0x0153,0x0187,
 0x001c,0x0055,0x008f,0x00c8,0x0102,0x013b,0x0175,0x01ae,
 0x001f,0x005e,0x009d,0x00dc,0x011c,0x015b,0x019a,0x01d9,
 0x0022,0x0067,0x00ad,0x00f2,0x0139,0x017e,0x01c4,0x0209,
 0x0026,0x0072,0x00bf,0x010b,0x0159,0x01a5,0x01f2,0x023e,
 0x002a,0x007e,0x00d2,0x0126,0x017b,0x01cf,0x0223,0x0277,
 0x002e,0x008a,0x00e7,0x0143,0x01a1,0x01fd,0x025a,0x02b6,
 0x0033,0x0099,0x00ff,0x0165,0x01cb,0x0231,0x0297,0x02fd,
 0x0038,0x00a8,0x0118,0x0188,0x01f9,0x0269,0x02d9,0x0349,
 0x003d,0x00b8,0x0134,0x01af,0x022b,0x02a6,0x0322,0x039d,
 0x0044,0x00cc,0x0154,0x01dc,0x0264,0x02ec,0x0374,0x03fc,
 0x004a,0x00df,0x0175,0x020a,0x02a0,0x0335,0x03cb,0x0460,
 0x0052,0x00f6,0x019b,0x023f,0x02e4,0x0388,0x042d,0x04d1,
 0x005a,0x010f,0x01c4,0x0279,0x032e,0x03e3,0x0498,0x054d,
 0x0063,0x012a,0x01f1,0x02b8,0x037f,0x0446,0x050d,0x05d4,
 0x006d,0x0148,0x0223,0x02fe,0x03d9,0x04b4,0x058f,0x066a,
 0x0078,0x0168,0x0259,0x0349,0x043b,0x052b,0x061c,0x070c,
 0x0084,0x018d,0x0296,0x039f,0x04a8,0x05b1,0x06ba,0x07c3,
 0x0091,0x01b4,0x02d8,0x03fb,0x051f,0x0642,0x0766,0x0889,
 0x00a0,0x01e0,0x0321,0x0461,0x05a2,0x06e2,0x0823,0x0963,
 0x00b0,0x0210,0x0371,0x04d1,0x0633,0x0793,0x08f4,0x0a54,
 0x00c2,0x0246,0x03ca,0x054e,0x06d2,0x0856,0x09da,0x0b5e,
 0x00d5,0x027f,0x042a,0x05d4,0x0780,0x092a,0x0ad5,0x0c7f,
 0x00ea,0x02bf,0x0495,0x066a,0x0840,0x0a15,0x0beb,0x0dc0,
 0x0102,0x0306,0x050b,0x070f,0x0914,0x0b18,0x0d1d,0x0f21,
 0x011c,0x0354,0x058c,0x07c4,0x09fc,0x0c34,0x0e6c,0x10a4,
 0x0138,0x03a8,0x0619,0x0889,0x0afb,0x0d6b,0x0fdc,0x124c,
 0x0157,0x0406,0x06b5,0x0964,0x0c14,0x0ec3,0x1172,0x1421,
 0x017a,0x046e,0x0762,0x0a56,0x0d4a,0x103e,0x1332,0x1626,
 0x019f,0x04de,0x081e,0x0b5d,0x0e9e,0x11dd,0x151d,0x185c,
 0x01c9,0x055c,0x08ef,0x0c82,0x1015,0x13a8,0x173b,0x1ace,
 0x01f7,0x05e5,0x09d4,0x0dc2,0x11b1,0x159f,0x198e,0x1d7c,
 0x0229,0x067c,0x0acf,0x0f22,0x1375,0x17c8,0x1c1b,0x206e,
 0x0260,0x0721,0x0be3,0x10a4,0x1567,0x1a28,0x1eea,0x23ab,
 0x029d,0x07d8,0x0d14,0x124f,0x178b,0x1cc6,0x2202,0x273d,
 0x02e0,0x08a1,0x0e63,0x1424,0x19e6,0x1fa7,0x2569,0x2b2a,
 0x032a,0x097f,0x0fd4,0x1629,0x1c7e,0x22d3,0x2928,0x2f7d,
 0x037b,0x0a72,0x1169,0x1860,0x1f57,0x264e,0x2d45,0x343c,
 0x03d4,0x0b7d,0x1326,0x1acf,0x2279,0x2a22,0x31cb,0x3974,
 0x0436,0x0ca3,0x1511,0x1d7e,0x25ec,0x2e59,0x36c7,0x3f34,
 0x04a2,0x0de7,0x172c,0x2071,0x29b7,0x32fc,0x3c41,0x4586,
 0x0519,0x0f4b,0x197e,0x23b0,0x2de3,0x3815,0x4248,0x4c7a,
 0x059b,0x10d2,0x1c0a,0x2741,0x327a,0x3db1,0x48e9,0x5420,
 0x062b,0x1281,0x1ed8,0x2b2e,0x3786,0x43dc,0x5033,0x5c89,
 0x06c9,0x145b,0x21ee,0x2f80,0x3d14,0x4aa6,0x5839,0x65cb,
 0x0777,0x1665,0x2553,0x3441,0x4330,0x521e,0x610c,0x6ffa,
 0x0836,0x18a2,0x290f,0x397b,0x49e8,0x5a54,0x6ac1,0x7b2d,
 0x0908,0x1b19,0x2d2a,0x3f3b,0x514c,0x635d,0x756e,0x877f,
 0x09ef,0x1dce,0x31ae,0x458d,0x596d,0x6d4c,0x812c,0x950b,
 0x0aee,0x20ca,0x36a6,0x4c82,0x625f,0x783b,0x8e17,0xa3f3,
 0x0c05,0x2410,0x3c1c,0x5427,0x6c34,0x843f,0x9c4b,0xb456,
 0x0d39,0x27ac,0x4220,0x5c93,0x7707,0x917a,0xabee,0xc661,
 0x0e8c,0x2ba4,0x48bd,0x65d5,0x82ee,0xa006,0xbd1f,0xda37,
 0x0fff,0x2ffe,0x4ffe,0x6ffd,0x8ffe,0xaffd,0xcffd,0xeffc
};

BOOL _BRVQAReaderReadSN2J(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	uint32_t size = BRPtrRangeGetDistance(r);
	if (size != 6)
		return NO;

	aVQAReader->adpcm.step_index = BRPtrRangeReadLE16AndAdvance(r) >> 5;
	aVQAReader->adpcm.predictor  = (int32_t)BRPtrRangeReadLE32AndAdvance(r);

	return YES;
}

BOOL _BRVQAReaderReadSND2(BRVQAReaderRef aVQAReader, BRPtrRangeRef r)
{
	uint32_t size = BRPtrRangeGetDistance(r);

	if (!aVQAReader->audioFrame || BRDataGetSize(aVQAReader->audioFrame) != 4 * size)
	{
		//printf("New audio frame size: %d\n", 4 * size);
		BRRelease(aVQAReader->audioFrame);
		aVQAReader->audioFrame = BRDataCreate(4 * size);
		assert(aVQAReader->audioFrame);
	}

	uint8_t *in  = BRPtrRangeGetBegin(r);
	uint8_t *end = in + size;

	int16_t *out = (int16_t*)BRDataGetBytes(aVQAReader->audioFrame);

	int16_t  step_index = aVQAReader->adpcm.step_index;
	int32_t  predictor  = aVQAReader->adpcm.predictor;

	while (in != end)
	{
		uint16_t bl = *in++;

		int n;
		for (n = 0; n != 2; ++n)
		{
			uint8_t nibble = (bl >> (4 * n)) & 0x0f;
			uint8_t code = nibble & 0x07;
			uint8_t sign = nibble & 0x08;

			int diff = ima_step_table[(step_index << 3) | code];

			// Westwood's IMA ADPCM differs from the below "standard" implementation
			// in the LSB is a couple of places.
			//int diff = ima_step_table_std[step_index] * code / 4 + ima_step_table_std[step_index] / 8;

			if (sign)
				predictor -= diff;
			else
				predictor += diff;

			predictor = MIN(MAX(predictor, -32768), 32767);

			*out++ = predictor;

			step_index = ima_index_table[code] + step_index;
			step_index = MIN(MAX(step_index, 0), 88);
		}
	}

	aVQAReader->adpcm.step_index = step_index;
	aVQAReader->adpcm.predictor  = predictor;

	BRPtrRangeAdvance(r, size);

	return YES;
}

BOOL BRVQAReaderReadFrame(BRVQAReaderRef aVQAReader, unsigned int aFrameNumber)
{
	BOOL rc = NO;
	BRPtrRangeRef r = 0;

	cleanup_if_not(aVQAReader);

	if (aFrameNumber >= aVQAReader->_header.numFrames)
		return NO;

	r = BRPtrRangeCreateCopy(aVQAReader->vqaRange);
	cleanup_if_not(r);

	BRPtrRangeAdvance(r, 2 * (aVQAReader->_frameInfo[aFrameNumber] & 0x1fffffff));

	if (!BRSizeIsEmpty(aVQAReader->_clipSize))
	{
		int frameSize = aVQAReader->_header.width * aVQAReader->_header.height;
		int i;
		for (i = 0; i != frameSize; ++i)
			aVQAReader->_frame[i] = 0xff;
	}

	uint32_t tag;
	do {
		tag = readTag(r);
		cleanup_if(BRPtrRangeGetDistance(r) < 4);
		uint32_t size = BRPtrRangeReadBE32AndAdvance(r);

		//printf("Tag: %s %d\n", str_tag(tag), size);

		cleanup_if(BRPtrRangeGetDistance(r) < size);
		LIMIT_SIZE(r, size);

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
				rc = _BRVQAReaderReadSN2J(aVQAReader, r);
				break;
			case kSND2:
				rc = _BRVQAReaderReadSND2(aVQAReader, r);
				break;
			default:
				puts("Unknown or unexpected tag:");
				hexdump(BRPtrRangeGetBegin(r) - 8, 16);
				BRPtrRangeAdvance(r, size);
		}

		UNDO_LIMIT_SIZE(r);
		cleanup_if_not(rc);
	} while (tag != kVQFR);

	free(r);
	return YES;
cleanup:
	free(r);
	return NO;
}

BRAVFrame BRVQAReaderGetAVFrame(BRVQAReaderRef aVQAReader)
{
	BRAVFrame frame;

	frame.video.size = BRSizeMake(aVQAReader->_header.width,
	                              aVQAReader->_header.height);

	frame.video.stride = BRSizeMake(aVQAReader->_frameStride,
	                                aVQAReader->_frameStride);

	frame.video.data = BRDataCreateWithBytes(
		4 * aVQAReader->_frameStride * aVQAReader->_frameStride,
		aVQAReader->_frame);

	frame.audio.data = 0;
	frame.audio.samples = 0;
	if (aVQAReader->audioFrame && BRDataGetSize(aVQAReader->audioFrame))
	{
		frame.audio.data    = BRDataCreateWithData(aVQAReader->audioFrame);
		frame.audio.samples = BRDataGetSize(frame.audio.data) / 2;
	}

	return frame;
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
	                               VQAReader->_header.blockH + 1;
	VQAReader->_codeBook  = malloc(VQAReader->_codeBookSize);

	VQAReader->_frameStride = roundUpToPowerOfTwo(
		MAX(VQAReader->_header.width, VQAReader->_header.height));

	VQAReader->_frame     = malloc(4 * VQAReader->_frameStride *
	                                   VQAReader->_frameStride);

	cleanup_if_not(VQAReader->_codeBook);
	cleanup_if_not(VQAReader->_frame);

	do {
		tag = readTag(r);
		//printf("Tag: %s\n", str_tag(tag));

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

	/*
	int i;
	for (i = 0; i != VQAReader->_loopCount; ++i)
	{
		printf("\tLoop %2d: %04x - %04x  %s\n", i,
			VQAReader->_loopInfo[i].frameBegin,
			VQAReader->_loopInfo[i].frameEnd,
			VQAReader->_loopInfo[i].name ? VQAReader->_loopInfo[i].name : "");
	}
	*/

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

BOOL BRVQAReaderGetLoop(BRVQAReaderRef aVQAReader, int aLoop, int *aFrameBegin, int *aFrameEnd)
{
	assert(aLoop < aVQAReader->_loopCount);

	*aFrameBegin = aVQAReader->_loopInfo[aLoop].frameBegin;
	*aFrameEnd   = aVQAReader->_loopInfo[aLoop].frameEnd;

	return YES;
}
