#include "BRUtils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

void hexdump(void *data, size_t size)
{
	unsigned char *p, *end;
	int c, c2;

	p = data;
	end = p + size;

	do {
		printf("%08x: ", (uintptr_t)(p - (unsigned char*)data));
		for (c = 0; c != 16 && p + c != end; ++c)
		{
			if (c == 8)
				putchar(' ');

			printf("%02x ", p[c]);
		}
		for (c2 = c; c2 != 16; ++c2)
		{
			if (c2 == 8)
				putchar(' ');

			printf("   ");
		}

		for (c = 0; c != 16 && p + c != end; ++c)
			putchar(isprint(p[c]) ? p[c] : '.');

		putchar('\n');

		p += c;
	} while (p != end);
	//putchar('\n');
}

BRSize BRSizeMake(unsigned int width, unsigned int height)
{
	BRSize s = { width, height };
	return s;
}

BOOL BRSizeIsEmpty(BRSize s)
{
	return s.width == 0 && s.height == 0;
}

uint32_t roundUpToPowerOfTwo(uint32_t x) {
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >>16);
	return x + 1;
}
