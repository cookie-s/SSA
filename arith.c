#include <stdint.h>
#include <assert.h>

#include "common.h"

static inline char allzero8(uint8_t *f, uint32_t n) {
    for(uint32_t i=0; i<n; i++)
        if(f[i]) return 0;
    return 1;
}
static inline char allzero64(uint8_t *ff, uint32_t n) {
    uint64_t *f = (uint64_t*)ff;
    n /= 8;
    for(uint32_t i=0; i<n; i++)
        if(f[i]) return 0;
    return 1;
}
static inline char add8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = t + f[i] + g[i];
        f[i] = t;
        t >>= 8;
    }
    if(t) {
        if(allzero8(f, n)) return 0;
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = t + f[i] + 0xFF;
            f[i] = t;
            t >>= 8;
        }
    }
    return 1;
}

static inline char add64(uint8_t *ff, const uint8_t *gg, uint32_t nn) {
    uint64_t *f = (uint64_t*)ff;
    const uint64_t *g = (uint64_t*)gg;
    const uint32_t  n = nn/8;
    uint128_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = t + f[i] + g[i];
        f[i] = t;
        t >>= 64;
    }
    if(t) {
        if(allzero64(ff, nn)) return 0;
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = t + f[i] + 0xFFFFFFFFFFFFFFFFULL;
            f[i] = t;
            t >>= 64;
        }
    }
    return 1;
}

extern inline char add(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        return add8(f, g, n);
    else
        return add64(f, g, n);
}

static inline uint8_t addc8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 0;
    for(uint32_t i=0; i<n; i++) {
        t = t + f[i] + g[i];
        f[i] = t;
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
        t = t + f[i] + g[i];
        f[i] = t;
        t >>= 64;
    }
    return t;
}

extern inline uint8_t addc(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        return addc8(f, g, n);
    else
        return addc64(f, g, n);
}

static inline char sub8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 2;
    for(uint32_t i=0; i<n; i++) {
        t = t + f[i] + (uint8_t)~g[i];
        f[i] = t;
        t >>= 8;
    }
    if(t) {
        if(allzero8(f, n)) return 0;
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = t + f[i] + 0xFF;
            f[i] = t;
            t >>= 8;
        }
    }
    return 1;
}
static inline char sub64(uint8_t *ff, const uint8_t *gg, uint32_t n) {
    uint64_t *f = (uint64_t*)ff;
    const uint64_t *g = (const uint64_t*)gg;
    n /= 8;

    uint128_t t = 2;
    for(uint32_t i=0; i<n; i++) {
        t = t + f[i] + (uint64_t)~g[i];
        f[i] = t;
        t >>= 64;
    }
    if(t) {
        if(allzero64(ff, n*8)) return 0;
        t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = t + f[i] + 0xFFFFFFFFFFFFFFFFULL;
            f[i] = t;
            t >>= 64;
        }
    }
    return 1;
}
extern inline char sub(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        return sub8(f, g, n);
    else
        return sub64(f, g, n);
}

static inline uint8_t subc8(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 1;
    for(uint32_t i=0; i<n; i++) {
        t = t + f[i] + (uint8_t)~g[i];
        f[i] = t;
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
        t = t + f[i] + (uint64_t)~g[i];
        f[i] = t;
        t >>= 64;
    }
    return t;
}

extern inline uint8_t subc(uint8_t *f, const uint8_t *g, uint32_t n) {
    if(n % 8)
        return subc8(f, g, n);
    else
        return subc64(f, g, n);
}
