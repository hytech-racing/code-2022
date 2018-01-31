#ifndef __XBTOOLS_H__
#define __XBTOOLS_H__

#include <stdint.h>
#include <stddef.h>

uint16_t fletcher16(const uint8_t *data, size_t len);
size_t cobs_encode(const uint8_t * input, size_t length, uint8_t * out);
size_t cobs_decode(const uint8_t * input, size_t length, uint8_t * out);

#endif