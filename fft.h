#pragma once

#include <stdint.h>

char fft (uint8_t *f, uint32_t u, uint32_t n);
char ifft (uint8_t *f, uint32_t u, uint32_t n);
char naive (uint8_t *f, uint32_t u, uint32_t n);
char inaive (uint8_t *f, uint32_t u, uint32_t n);
