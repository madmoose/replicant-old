#include "BREndian.h"

#include <stdint.h>

#define BR_BIG_ENDIAN 0

uint16_t swap16(uint16_t a)
{
	return ((a & 0x00ff) <<  8)
	     | ((a & 0xff00) >>  8);
}

uint32_t swap32(uint32_t a)
{
	return ((a & 0x000000ff) << 24)
	     | ((a & 0x0000ff00) <<  8)
	     | ((a & 0x00ff0000) >>  8)
	     | ((a & 0xff000000) >> 24);
}

#if BR_BIG_ENDIAN
uint16_t letoh16(uint16_t a) { return swap16(a); }
uint16_t htole16(uint16_t a) { return swap16(a); }

uint32_t letoh32(uint32_t a) { return swap32(a); }
uint32_t htole32(uint32_t a) { return swap32(a); }

uint16_t betoh16(uint16_t a) { return a; }
uint16_t htobe16(uint16_t a) { return a; }

uint32_t betoh32(uint32_t a) { return a; }
uint32_t htobe32(uint32_t a) { return a; }

#else

uint16_t letoh16(uint16_t a) { return a; }
uint16_t htole16(uint16_t a) { return a; }

uint32_t letoh32(uint32_t a) { return a; }
uint32_t htole32(uint32_t a) { return a; }

uint16_t betoh16(uint16_t a) { return swap16(a); }
uint16_t htobe16(uint16_t a) { return swap16(a); }

uint32_t betoh32(uint32_t a) { return swap32(a); }
uint32_t htobe32(uint32_t a) { return swap32(a); }
#endif
