#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef unsigned __int128 uint128_t;

void print_hex(uint8_t *f, uint32_t n) {
    for(int i=0; i<n; i+=64) {
        for(int j=0; j<64; j+=16) {
            for(int k=0; k<16 && i+j+k<n; k++)
                printf("%02x", f[i+j+k]);
            printf(" ");
        }
        puts("");
    }
    return;
}

inline uint32_t bitrev32(uint32_t x)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}

inline uint32_t bitrev(uint32_t x, int msbi) {
    return bitrev32(x) >> (msbi+1);
}

inline void add(uint8_t *f, const uint8_t *g, uint32_t n) {
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
inline void sub(uint8_t *f, const uint8_t *g, uint32_t n) {
    uint16_t t = 2;
    for(uint32_t i=0; i<n; i++) {
        t = (uint16_t)f[i] + (uint16_t)((~g[i] & 0xFF)) + t;
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

void shift(uint8_t *f, int m, uint32_t n) {
    // f * (1<<m) mod (1<<8*n + 1);

    m %= 2*8*n;
    m += 2*8*n;
    m %= 2*8*n;
    if(m >= 8*n) shift(f, m - (8*n), n), m = 8*n;

    {
        uint16_t t = 0;
        for(uint32_t i=0; i<n; i++) {
            t = (((uint16_t)f[i]) << (m%8)) | t;
            f[i]  = (uint8_t)t;
            t >>= 8;
        }
        if(t) {
            t = (~t + 1) & 0xFF;
            t = (uint16_t)f[0] + t;
            f[0] = (uint8_t)t;
            t >>= 8;
            for(int i=1; i<n; i++) {
                t = (uint16_t)f[i] + 0xFF + t;
                f[i] = (uint8_t)t;
                t >>= 8;
            }
        }
    }

    m /= 8;
    if(!m) return;

    {
        uint8_t *flag = (uint8_t*)malloc(n);
        memset(flag, 0, n);
        for(uint32_t i=0; i<m; i++) {
            uint8_t s = f[i];
            for(int j=i+m; !flag[j%n]; j+=m) {
                if(j>=n) s = ~s, j%=n;
                uint8_t t = f[j];
                f[j] = s;
                flag[j] = 1;
                s = t;
            }
        }
        free(flag); flag = 0;

        char zero = 1;
        for(uint32_t i=0; zero && i<m; i++)
            if(f[i] != 0xFF) zero = 0;
        if(zero) {
            for(uint32_t i=0; i<m; i++)
                f[i] = ~f[i];
            return;
        }

        uint16_t t = 1;
        for(uint32_t i=0; i<n; i++) {
            t = (uint16_t)f[i] + (i >= m ? 0xFF : 0) + t;
            f[i] = (uint8_t)t;
            t >>= 8;
        }

        if(!t) {
            t = 1;
            for(uint32_t i=0; i<n; i++) {
                t = (uint16_t)f[i] + t;
                f[i] = (uint8_t)t;
                t >>= 8;
            }
        }
    }
}

inline void fft (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    const int msbi = __builtin_clz(n);
    uint8_t *a = (uint8_t*)alloca(u * sizeof(uint8_t));
    uint8_t *b = (uint8_t*)alloca(u * sizeof(uint8_t));
    for(uint32_t i=1,ilog=0; i<n; i<<=1,ilog++) {
        const uint32_t wlog = n>>ilog;
        for(uint32_t j=0; j<n; j+=i<<1) {
            uint32_t wnlog = 0;
            for(uint32_t k=0; k<i; k++) {
                const uint32_t ai = bitrev(j+k, msbi);
                const uint32_t bi = bitrev(i+j+k, msbi);

                memcpy(a, f + u*ai, u * sizeof(uint8_t));
                memcpy(b, f + u*bi, u * sizeof(uint8_t));
                shift(b, wnlog * (u*8/n), u);
                add(f + u*ai, b, u);
                sub(a, b, u);
                memcpy(f + u*bi, a, u * sizeof(uint8_t));
                wnlog += wlog;
                wnlog %= 2*8*u;
            }
        }
    }
    for(uint32_t i=0; i<n; i++) {
        const uint32_t j = bitrev(i,msbi);
        if(j>=i) continue;
        memcpy(a, f + u*i, u * sizeof(uint8_t));
        memcpy(f + u*i, f + u*j, u * sizeof(uint8_t));
        memcpy(f + u*j, a, u * sizeof(uint8_t));
    }
    return;
}

inline void ifft (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    const int msbi = __builtin_clz(n);
    uint8_t *a = (uint8_t*)alloca(u * sizeof(uint8_t));
    uint8_t *b = (uint8_t*)alloca(u * sizeof(uint8_t));
    for(uint32_t i=1,ilog=0; i<n; i<<=1,ilog++) {
        const uint32_t wlog = (2*8*u-1) * (n>>ilog);
        for(uint32_t j=0; j<n; j+=i<<1) {
            uint32_t wnlog = 0;
            for(uint32_t k=0; k<i; k++) {
                const uint32_t ai = bitrev(j+k, msbi);
                const uint32_t bi = bitrev(i+j+k, msbi);

                memcpy(a, f + u*ai, u * sizeof(uint8_t));
                memcpy(b, f + u*bi, u * sizeof(uint8_t));
                shift(b, wnlog * (u*8/n), u);
                add(f + u*ai, b, u);
                sub(a, b, u);
                memcpy(f + u*bi, a, u * sizeof(uint8_t));
                wnlog += wlog;
                wnlog %= 2*8*u;
            }
        }
    }
    for(uint32_t i=0; i<n; i++) {
        const uint32_t j = bitrev(i,msbi);
        if(j>=i) continue;
        memcpy(a, f + u*i, u * sizeof(uint8_t));
        memcpy(f + u*i, f + u*j, u * sizeof(uint8_t));
        memcpy(f + u*j, a, u * sizeof(uint8_t));
    }

    const uint32_t nlog = __builtin_ctz(n);
    for(uint32_t i=0; i<n; i++)
        shift(f + i*u, -(int)nlog, u);
    return;
}

inline void naive (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    uint8_t *F = (uint8_t*)alloca(u*n);
    uint8_t *buf = (uint8_t*)alloca(u);
    memset(F, 0, u*n);
    for(uint32_t i=0; i<n; i++) {
        for(uint32_t j=0; j<n; j++) {
            memcpy(buf, f + u*j, u);
            shift(buf, (int)(2*i*j*(u*8/n)), u); // mod 2^(8*u) + 1
            add(F + u*i, buf, u);
        }
    }
    memcpy(f, F, u*n);
}

inline void inaive (uint8_t *f, uint32_t u, uint32_t n) {
    assert(u*8 >= n);
    uint8_t *F = (uint8_t*)alloca(u * n);
    uint8_t *buf = (uint8_t*)alloca(u);
    memset(F, 0, u*n);
    for(uint32_t i=0; i<n; i++) {
        for(uint32_t j=0; j<n; j++) {
            memcpy(buf, f + u*j, u);
            shift(buf, -(int)(2*i*j*(u*8/n)) , u);
            add(F + u*i, buf, u);
        }
    }
    const int nlog = __builtin_ctz(n);
    for(uint32_t i=0; i<n; i++)
        shift(F + i*u, -nlog, u);

    memcpy(f, F, u*n);
}

void classical(uint8_t *h, const uint8_t *f, const uint8_t *g, uint32_t n) {
    memset(h, 0, 2*n * sizeof(uint8_t));
    for(size_t i=0; i<n; i++) {
        for(size_t j=0; j<n; j++) {
            uint16_t t = (uint16_t)f[i] * (uint16_t)g[j];
            for(int k=i+j; k<2*n && t; k++) {
                if(((uint16_t)h[k] + (t & ((1<<8)-1)))>>8)
                    t += 1<<8;
                h[k] += (uint8_t)t;
                t >>= 8;
            }
        }
    }
}
inline void classical64(uint8_t *hh, const uint8_t *ff, const uint8_t *gg, uint32_t nn) {
    assert(nn % 8 == 0);
    memset(hh, 0, 2*nn * sizeof(uint8_t));
    const uint64_t *f = (const uint64_t*)ff;
    const uint64_t *g = (const uint64_t*)gg;
    const uint64_t  n = nn/8;
    uint64_t *h = (uint64_t*)hh;
    const uint128_t one = 1;
    for(size_t i=0; i<n; i++) {
        for(size_t j=0; j<n; j++) {
            uint128_t t = (uint128_t)f[i] * (uint128_t)g[j];
            for(int k=i+j; k<2*n && t; k++) {
                if(((uint128_t)h[k] + (t & ((one<<64)-1)))>>64)
                    t += one<<64;
                h[k] += (uint128_t)t;
                t >>= 64;
            }
        }
    }
}

void mult(uint8_t *h, const uint8_t *f, const uint8_t *g, uint32_t n) {
    // len(h) == 2 * len(f) == 2 * len(g) == 2 * n
    if (n <= 4096) {
        classical64(h, f, g, n);
        return;
    }

    const uint32_t k = 10; // param
    const uint32_t sp = 1<<k;
    uint32_t u;

    //for(u = 2; !(sp-(k-1)/2 < n/sp); u++);
    u = 2*n/sp;
    u*= 2;

    //printf("k%d u%d sp%d n%d\n", k, u, sp, n);
    uint8_t *ff = (uint8_t*)malloc(2*u*sp * sizeof(uint8_t));
    uint8_t *gg = (uint8_t*)malloc(2*u*sp * sizeof(uint8_t));
    memset(ff, 0, 2*u*sp * sizeof(uint8_t));
    memset(gg, 0, 2*u*sp * sizeof(uint8_t));

    const uint32_t each = n/sp;
    for(int i=0; i<sp; i++) {
        for(int j=0; j<each; j++) {
            ff[i*u+j] = f[i*each+j];
            gg[i*u+j] = g[i*each+j];
        }
    }
    fft(ff, u, 2*sp);
    fft(gg, u, 2*sp);

    uint8_t *hh = (uint8_t*)malloc(2*u * sizeof(uint8_t));
    for(uint32_t i=0; i<2*sp; i++) {
        assert(u < n);
        mult(hh, ff + u*i, gg + u*i, u);
        sub(hh, hh + u, u);
        memcpy(ff + u*i, hh, u * sizeof(uint8_t));
    }
    free(hh); hh = 0;

    ifft(ff, u, 2*sp);
    memset(gg, 0, 2*u);
    memset(gg+2*u, 0, 2*u);
    for(int i=0; i<2*sp; i++) {
        memcpy(gg + 2*u, ff + i*u, u);
        add(gg, gg + 2*u, 2*u);
        for(int j=0; j<each; j++) {
            h[i*each+j] = gg[j];
            gg[j] = 0;
        }
        shift(gg, -(8*each), 2*u);
    }
    free(ff); ff = 0;
    free(gg); gg = 0;
}
