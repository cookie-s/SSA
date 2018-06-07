#include <stdint.h>

#include "common.h"

static inline void add8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = (uint16_t)f[i] + (uint16_t)g[i] + t;
        f[i] = (uint8_t)t;
        t >>= 8;
    }
    if(t) {
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = (uint16_t)f[i] + 0xFF + t;
            f[i] = (uint8_t)t;
            t >>= 8;
        }
    }
}

static inline void add64(uint8_t *ff, const uint8_t *gg, uint32_t nn) {
    uint64_t *f = (uint64_t*)ff;
    const uint64_t *g = (uint64_t*)gg;
    const uint32_t  n = nn/8;
    uint128_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = (uint128_t)f[i] + (uint128_t)g[i] + t;
        f[i] = (uint64_t)t;
        t >>= 64;
    }
    if(t) {
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = (uint128_t)f[i] + 0xFFFFFFFFFFFFFFFFULL + t;
            f[i] = (uint64_t)t;
            t >>= 64;
        }
    }
}

void add(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        add8(f, g, n);
    else
        add64(f, g, n);
}

static inline uint8_t addc8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = (uint16_t)f[i] + (uint16_t)g[i] + t;
        f[i] = (uint8_t)t;
        t >>= 8;
    }
    return t;
}

static inline uint8_t addc64(uint8_t *ff, const uint8_t *gg, uint32_t nn) {
    uint64_t *f = (uint64_t*)ff;
    const uint64_t *g = (uint64_t*)gg;
    const uint32_t  n = nn/8;
    uint128_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = (uint128_t)f[i] + (uint128_t)g[i] + t;
        f[i] = (uint64_t)t;
        t >>= 64;
    }
    return (uint8_t)t;
}

uint8_t addc(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        return addc8(f, g, n);
    else
        return addc64(f, g, n);
}

static inline void sub8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 2;
    for(uint32_t i=0; i<n; i++) {
        t = (uint16_t)f[i] + (uint16_t)(~g[i] & 0xFF) + t;
        f[i] = (uint8_t)t;
        t >>= 8;
    }
    if(t) {
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = (uint16_t)f[i] + 0xFF + t;
            f[i] = (uint8_t)t;
            t >>= 8;
        }
    }
}
static inline void sub64(uint8_t *ff, const uint8_t *gg, uint32_t n) {
    uint64_t *f = (uint64_t*)ff;
    const uint64_t *g = (const uint64_t*)gg;
    n /= 8;

    uint128_t t = 2;
    for(uint32_t i=0; i<n; i++) {
        t = (uint128_t)f[i] + (uint64_t)~g[i] + t;
        f[i] = (uint64_t)t;
        t >>= 64;
    }
    if(t) {
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = (uint128_t)f[i] + 0xFFFFFFFFFFFFFFFFULL + t;
            f[i] = (uint64_t)t;
            t >>= 64;
        }
    }
}
void sub(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        sub8(f, g, n);
    else
        sub64(f, g, n);
}

static inline uint8_t subc8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 1;
    for(uint32_t i=0; i<n; i++) {
        t = (uint16_t)f[i] + (uint8_t)~g[i] + t;
        f[i] = (uint8_t)t;
        t >>= 8;
    }
    return t;
}
static inline uint8_t subc64(uint8_t *ff, const uint8_t *gg, uint32_t n) {
    uint64_t *f = (uint64_t*)ff;
    const uint64_t *g = (const uint64_t*)gg;
    n /= 8;

    uint128_t t = 1;
    for(uint32_t i=0; i<n; i++) {
        t = (uint128_t)f[i] + (uint64_t)~g[i] + t;
        f[i] = (uint64_t)t;
        t >>= 64;
    }
    return t;
}

uint8_t subc(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        return subc8(f, g, n);
    else
        return subc64(f, g, n);
}
