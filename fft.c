#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "arith.h"

static inline uint32_t bitrev32(uint32_t x)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}

static inline uint32_t bitrev(uint32_t x, int msbi) {
    return bitrev32(x) >> (msbi+1);
}

static inline uint32_t cntbits8(uint8_t *f, uint32_t n) {
    uint32_t res = 0;
    for(uint32_t i=0; i<n; i++)
        res += __builtin_popcount(f[i]);
    return res;
}
static inline uint32_t cntbits64(uint8_t *ff, uint32_t n) {
    uint64_t *f = (uint64_t*)ff;
    n /= 8;
    uint32_t res = 0;
    for(uint32_t i=0; i<n; i++)
        res += __builtin_popcountll(f[i]);
    return res;
}

static inline char shift8(uint8_t *f, int m, uint32_t n) {
    // f * (1<<m) mod (1<<8*n + 1);

    m %= 2*8*n;
    m += 2*8*n;
    m %= 2*8*n;

    uint32_t cnt = cntbits8(f, n);
    if(cnt == 0) return 1;

    {
        uint16_t t = 0;
        for(uint32_t i=0; i<n; i++) {
            t |= ((uint16_t)f[i] << (m%8));
            f[i]  = t;
            t >>= 8;
        }
        if(cnt == 1 && t == 1) return 0;
        if(t) {
            t = (uint8_t)(~t + 1);
            t = f[0] + t;
            f[0] = t;
            t >>= 8;
            for(int i=1; i<n; i++) {
                t = t + f[i] + 0xFF;
                f[i] = t;
                t >>= 8;
            }
            if(!t) {
                t = 1;
                for(int i=0; i<n; i++) {
                    t = t + f[i];
                    f[i] = t;
                    t >>= 8;
                }
            }
        }
    }

    m /= 8;
    if(!m) return 1;
    if(cnt == 1 && m < n && f[n-m] == 1) return 0;

    {
        uint8_t *flag = (uint8_t*)alloca(n);
        memset(flag, 0, n);
        for(uint32_t i=0; i<n; i++) {
            uint8_t s = f[i];
            for(int j=(i+m)%n; !flag[j]; j+=m,j%=n) {
                uint8_t t = f[j];
                f[j] = s;
                flag[j] = 1;
                s = t;
            }
        }

        uint16_t t = 1;
        for(uint32_t i=0; i<n; i++) {
            t = ((i-m+2*n)%(2*n) >= n ? (uint8_t)~f[i] : (0xFF+f[i])) + t;
            f[i] = t;
            t >>= 8;
        }

        if(!t) {
            t = 1;
            for(uint32_t i=0; i<n; i++) {
                t = f[i] + t;
                f[i] = t;
                t >>= 8;
            }
        }
    }
    return 1;
}

static inline char shift64(uint8_t *ff, int m, uint32_t n) {
    assert(n % 8 == 0);
    // f * (1<<m) mod (1<<8*n + 1);

    uint128_t max = 0xFFFFFFFFFFFFFFFFULL;
    uint32_t cnt = cntbits64(ff, n);
    if(cnt == 0) return 1;
    uint64_t *f = (uint64_t*)ff;

    m %= 2*8*n;
    m += 2*8*n;
    m %= 2*8*n;
    n /= 8;

    {
        uint128_t t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = (((uint128_t)f[i]) << (m%64)) | t;
            f[i]  = t;
            t >>= 64;
        }
        if(cnt == 1 && t == 1) return 0;
        if(t) {
            t = (uint64_t)-t;
            t = f[0] + t;
            f[0] = t;
            t >>= 64;
            for(int i=1; i<n; i++) {
                t = t + f[i] + max;
                f[i] = t;
                t >>= 64;
            }
            if(!t) {
                t = 1;
                for(int i=0; i<n; i++) {
                    t = f[i] + t;
                    f[i] = t;
                    t >>= 64;
                }
            }
        }
    }

    m /= 64;
    if(!m) return 1;
    if(cnt == 1 && m < n && f[n-m] == 1) return 0;

    {
        uint8_t *flag = (uint8_t*)alloca(n);
        memset(flag, 0, n);
        for(uint32_t i=0; i<n; i++) {
            uint64_t s = f[i];
            for(int j=(i+m)%n; !flag[j]; j+=m,j%=n) {
                uint64_t t = f[j];
                f[j] = s;
                flag[j] = 1;
                s = t;
            }
        }

        uint128_t t = 1;
        for(uint32_t i=0; i<n; i++) {
            t = ((i-m+2*n)%(2*n) >= n ? (uint64_t)~f[i] : (max+f[i])) + t;
            f[i] = t;
            t >>= 64;
        }

        if(!t) {
            t = 1;
            for(uint32_t i=0; i<n; i++) {
                t = f[i] + t;
                f[i] = t;
                t >>= 64;
            }
        }
    }
    return 1;
}
static inline char shift(uint8_t *f, uint32_t m, uint32_t n) {
    if(n % 8 == 0)
        return shift64(f, m, n);
    else
        return shift8(f, m, n);
}

extern inline char fft (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    const int msbi = __builtin_clz(n);
    uint8_t *a = (uint8_t*)alloca(u);
    uint8_t *b = (uint8_t*)alloca(u);
    for(uint32_t i=1,ilog=0; i<n; i<<=1,ilog++) {
        const uint32_t wlog = n>>ilog;
        for(uint32_t j=0; j<n; j+=i<<1) {
            uint32_t wnlog = 0;
            for(uint32_t k=0; k<i; k++) {
                const uint32_t ai = bitrev(j+k, msbi);
                const uint32_t bi = bitrev(i+j+k, msbi);

                memcpy(a, f + u*ai, u);
                memcpy(b, f + u*bi, u);
                if(!shift(b, wnlog * (u*8/n), u)) return 0;
                if(!add(f + u*ai, b, u)) return 0;
                if(!sub(a, b, u)) return 0;
                memcpy(f + u*bi, a, u);
                wnlog += wlog;
                wnlog %= 2*8*u;
            }
        }
    }
    for(uint32_t i=0; i<n; i++) {
        const uint32_t j = bitrev(i,msbi);
        if(j>=i) continue;
        memcpy(a, f + u*i, u);
        memcpy(f + u*i, f + u*j, u);
        memcpy(f + u*j, a, u);
    }
    return 1;
}

extern inline char ifft (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    const int msbi = __builtin_clz(n);
    uint8_t *a = (uint8_t*)alloca(u);
    uint8_t *b = (uint8_t*)alloca(u);
    for(uint32_t i=1,ilog=0; i<n; i<<=1,ilog++) {
        const uint32_t wlog = (2*8*u-1) * (n>>ilog);
        for(uint32_t j=0; j<n; j+=i<<1) {
            uint32_t wnlog = 0;
            for(uint32_t k=0; k<i; k++) {
                const uint32_t ai = bitrev(j+k, msbi);
                const uint32_t bi = bitrev(i+j+k, msbi);

                memcpy(a, f + u*ai, u);
                memcpy(b, f + u*bi, u);
                if(!shift(b, wnlog * (u*8/n), u)) return 0;
                if(!add(f + u*ai, b, u)) return 0;
                if(!sub(a, b, u)) return 0;
                memcpy(f + u*bi, a, u);
                wnlog += wlog;
                wnlog %= 2*8*u;
            }
        }
    }
    for(uint32_t i=0; i<n; i++) {
        const uint32_t j = bitrev(i,msbi);
        if(j>=i) continue;
        memcpy(a, f + u*i, u);
        memcpy(f + u*i, f + u*j, u);
        memcpy(f + u*j, a, u);
    }

    const uint32_t nlog = __builtin_ctz(n);
    for(uint32_t i=0; i<n; i++)
        if(!shift(f + i*u, -(int)nlog, u)) return 0;
    return 1;
}

extern inline char naive (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    uint8_t *F = (uint8_t*)alloca(u*n);
    uint8_t *buf = (uint8_t*)alloca(u);
    memset(F, 0, u*n);
    for(uint32_t i=0; i<n; i++) {
        for(uint32_t j=0; j<n; j++) {
            memcpy(buf, f + u*j, u);
            if(!shift(buf, (int)(2*i*j*(u*8/n)), u)) return 0; // mod 2^(8*u) + 1
            if(!add(F + u*i, buf, u)) return 0;
        }
    }
    memcpy(f, F, u*n);
    return 1;
}

extern inline char inaive (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    uint8_t *F = (uint8_t*)alloca(u * n);
    uint8_t *buf = (uint8_t*)alloca(u);
    memset(F, 0, u*n);
    for(uint32_t i=0; i<n; i++) {
        for(uint32_t j=0; j<n; j++) {
            memcpy(buf, f + u*j, u);
            if(!shift(buf, -(int)(2*i*j*(u*8/n)) , u)) return 0;
            if(!add(F + u*i, buf, u)) return 0;
        }
    }
    const int nlog = __builtin_ctz(n);
    for(uint32_t i=0; i<n; i++)
        if(!shift(F + i*u, -nlog, u)) return 0;

    memcpy(f, F, u*n);
    return 1;
}
