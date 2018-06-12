#pragma once

#include <stdint.h>

void classical(uint8_t *h, const uint8_t *f, const uint8_t *g, uint32_t n);
void classical64(uint8_t *hh, const uint8_t *ff, const uint8_t *gg, uint32_t nn);
void karatsuba(uint8_t *h, const uint8_t *f, const uint8_t *g, uint32_t n);
char mult(uint8_t *h, const uint8_t *f, const uint8_t *g, uint32_t n);
char _mult(uint8_t *h, const uint8_t *f, const uint8_t *g, uint32_t n, uint32_t k);
