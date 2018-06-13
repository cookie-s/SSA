#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "arith.h"
#include "fft.h"
#include "mult.h"

void classical(uint8_t * restrict h, const uint8_t *f, const uint8_t *g, uint32_t n) {
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
extern inline void classical64(uint8_t * restrict hh, const uint8_t *ff, const uint8_t *gg, uint32_t nn) {
    assert(nn % 8 == 0);
    memset(hh, 0, 2*nn * sizeof(uint8_t));
    const uint64_t *f = (const uint64_t*)ff;
    const uint64_t *g = (const uint64_t*)gg;
    const uint64_t  n = nn/8;
    uint64_t *h = (uint64_t*)hh;
    for(size_t i=0; i<n; i++) {
        for(size_t j=0; j<n; j++) {
            uint128_t t = (uint128_t)f[i] * (uint128_t)g[j];
            for(int k=i+j; k<2*n && t; k++) {
                t += (((uint128_t)h[k] + (uint64_t)t)>>64)<<64;
                h[k] += (uint64_t)t;
                t >>= 64;
            }
        }
    }
}
void karatsuba(uint8_t * restrict h, const uint8_t *f, const uint8_t *g, uint32_t n) {
    assert(n % 2 == 0);
    memset(h, 0, 2*n);
    if(n <= 32) {
        classical64(h, f, g, n);
        return;
    }
    const uint32_t m = n/2;
    uint8_t *buf = malloc(n + 2*n + n + n);
    uint8_t *z0 = buf;
    uint8_t *z1 = buf + n;
    uint8_t *z2 = buf + n + 2*n;
    uint8_t *b1 = buf + n + 2*n + n;
    uint8_t *b2 = buf + n + 2*n + n + m;
    uint8_t flag = 0;
    karatsuba(z0, f, g, m);
    karatsuba(z2, f+m, g+m, m);

    memcpy(b1, f+m, m);
    memcpy(b2, g+m, m);
    flag |= addc(b1, f, m) << 0;
    flag |= addc(b2, g, m) << 1;
    karatsuba(z1, b1, b2, m);
    if(flag & 1) h[3*m] += addc(z1+m, b2, m);
    if(flag & 2) h[3*m] += addc(z1+m, b1, m);
    h[3*m] += flag == 0b11;
    if(!subc(z1, z0, n)) assert(h[3*m]), h[3*m]--;
    if(!subc(z1, z2, n)) assert(h[3*m]), h[3*m]--;

    memcpy(h, z0, n);
    h[3*m] += addc(h + m, z1, n);
    assert(!addc(h + 2*m, z2, n));

    free(buf);
}

static inline uint32_t optk(uint32_t n) {
    switch(n) {
        case 1<<7:
        case 1<<8:
        case 1<<9:
        case 1<<10:
            return 10;
        case 1<<11:
            return 11;
        case 1<<12:
            return 12;
        case 1<<13:
            return 6;
        case 1<<14:
            return 6;
        case 1<<15:
            return 9;
        case 1<<16:
            return 10;
        case 1<<17:
            return 10;
        case 1<<18:
            return 11;
        case 1<<19:
            return 11;
        case 1<<20:
            return 12;
        case 1<<21:
            return 12;
        case 1<<22:
            return 13;
        case 1<<23:
            return 13;
        case 1<<24:
            return 14;
        case 1<<25:
            return 12;
        default:
            return 14;
    }
}
char _mult(uint8_t * restrict h, const uint8_t *f, const uint8_t *g, uint32_t n, uint32_t k);
char mult(uint8_t * restrict h, const uint8_t *f, const uint8_t *g, uint32_t n) {
    return _mult(h,f,g,n,optk(n));
}
char _mult(uint8_t * restrict h, const uint8_t *f, const uint8_t *g, uint32_t n, uint32_t k) {
    // len(h) == 2 * len(f) == 2 * len(g) == 2 * n

    uint32_t sp, u, each;
    for(;k<18; k+=2) {
        sp = 1<<k;

        if (n <= sp) {
            karatsuba(h, f, g, n);
            return 1;
        }

        each = n/sp;
        for(u=2; !(u*8 >= 2*sp && 2*8*each+k-1<=8*u); u*=2);


        uint8_t *buf = (uint8_t*)malloc(2*u*sp * 2 + 2*u * 2);
        memset(buf, 0, 2*u*sp * 2);
        uint8_t *ff = buf;
        uint8_t *gg = buf + 2*u*sp;
        uint8_t *hh = buf + 2*u*sp * 2;
        uint8_t *hh2 = buf + 2*u*sp * 2 + 2*u;

        for(int i=0; i<sp; i++) {
            memcpy(ff + i*u, f + i*each, each);
            memcpy(gg + i*u, g + i*each, each);
        }
        if(!fft(ff, u, 2*sp)) goto KINC;
        if(!fft(gg, u, 2*sp)) goto KINC;

        for(uint32_t i=0; i<2*sp; i++) {
            assert(u < n);
            if(!mult(hh, ff + u*i, gg + u*i, u)) goto KINC;
#if 0
            karatsuba(hh2, ff + u*i, gg + u*i, u);
            if(memcmp(hh,hh2,2*u)) {
                printf("k%d sp%d u%d e%d\n", k, sp, u, each);
                print_hex(ff + u*i, u); puts("f");
                print_hex(gg + u*i, u); puts("g");
                print_hex(hh, 2*u); puts("hh");
                print_hex(hh2, 2*u); puts("hh2");
                assert(!memcmp(hh, hh2, 2*u));
            }
#endif
            sub(hh, hh + u, u);
            memcpy(ff + u*i, hh, u * sizeof(uint8_t));
        }
        if(!ifft(ff, u, 2*sp)) goto KINC;

        memset(gg, 0, 2*u);
        memset(gg+2*u+u, 0, u);
        for(int i=0; i<2*sp; i++) {
            memcpy(gg + 2*u, ff + i*u, u);
            if(!add(gg, gg + 2*u, 2*u)) continue;
            memcpy(h + i*each, gg, each);
            memmove(gg, gg+each, 2*u-each);
            memset(gg + 2*u-each, 0, each);
        }
        free(buf);
        return 1;

KINC:
        ;
    }
    return 0;
}
