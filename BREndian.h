#ifndef BR_ENDIAN_H
#define BR_ENDIAN_H

#include <stdint.h>
#include <string.h>

uint16_t letoh16(uint16_t a);
uint16_t htole16(uint16_t a);

uint32_t letoh32(uint32_t a);
uint32_t htole32(uint32_t a);

uint16_t betoh16(uint16_t a);
uint16_t htobe16(uint16_t a);

uint32_t betoh32(uint32_t a);
uint32_t htobe32(uint32_t a);

#endif
