#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "fft.h"
#include "mult.h"
#include "arith.h"

const uint8_t a[] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const uint8_t b[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
const uint8_t c[] = {5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0};
const uint8_t d[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
const uint8_t e[] = {0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24};
#define U (sizeof(a)/sizeof(uint8_t))

void test_add() {
    uint8_t buf1[U];

    {
        memcpy(buf1, a, U);
        add(buf1, a, U);
        const uint8_t buf2[] = {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        assert(!memcmp(buf1, buf2, U));
    }
    {
        memcpy(buf1, d, U);
        add(buf1, d, U);
        const uint8_t buf2[] = {0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        assert(!memcmp(buf1, buf2, U));
    }

    {
        memcpy(buf1, c, U);
        add(buf1, c, U);
        const uint8_t buf2[] = {0x0a,0x08,0x06,0x04,0x02,0x00,0x12,0x10,0x0e,0x0c,0x0a,0x08,0x06,0x04,0x02,0x00};
        assert(!memcmp(buf1, buf2, U));
    }
}

void test_sub() {
    uint8_t buf1[U];

    {
        memcpy(buf1, a, U);
        sub(buf1, a, U);
        add(buf1, a, U);
        assert(!memcmp(buf1, a, U));
    }
    {
        memcpy(buf1, d, U);
        sub(buf1, c, U);
        add(buf1, c, U);
        assert(!memcmp(buf1, d, U));
    }
    {
        memcpy(buf1, c, U);
        sub(buf1, d, U);
        add(buf1, d, U);
        assert(!memcmp(buf1, c, U));
    }
    {
        memcpy(buf1, d, U);
        sub(buf1, e, U);
        add(buf1, e, U);
        assert(!memcmp(buf1, d, U));
    }
}

void test_shift() {
    uint8_t buf1[U];
    char shift(uint8_t *f, uint32_t m, uint32_t n);

    {
        memcpy(buf1, c, U);
        shift(buf1, 0, U);
        assert(!memcmp(buf1, c, U));
    }
    {
        memcpy(buf1, d, U);
        shift(buf1, 5, U);
        const uint8_t buf2[] = {0xC1,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        assert(!memcmp(buf1, buf2, U));
    }
    {
        memcpy(buf1, d, U);
        shift(buf1, 25, U);
        const uint8_t buf2[] = {0x01,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        assert(!memcmp(buf1, buf2, U));
    }
    {
        memcpy(buf1, c, U);
        shift(buf1, 67, U);
        const uint8_t buf2[] = {0xc8,0xcf,0xd7,0xdf,0xe7,0xef,0xf7,0xff,0x27,0x20,0x18,0x10,0x08,0x00,0x48,0x40};
        assert(!memcmp(buf1, buf2, U));
    }
    for(int i=0; i<2*8*U; i++) {
        uint8_t buf2[U];
        memcpy(buf1, e, U);
        memcpy(buf2, buf1, U);
        shift(buf1,  i, U);
        shift(buf1, -i, U);
        assert(!memcmp(buf1, buf2, U));
    }
}

void test_fft() {
    uint8_t buf1[U];
    {
        memcpy(buf1, c, U);
        add(buf1, b, U);
        add(buf1, e, U);
        add(buf1, e, U);
        uint8_t buf2[4*U] = {};
        for(int i=0; i<U; i++)
            buf2[i*4] = buf1[i];
        fft(buf2, 4, U);
        ifft(buf2, 4, U);
        for(int i=0; i<U; i++)
            buf2[i] = buf2[4*i];
        assert(!memcmp(buf1, buf2, U));
    }
    {
        memcpy(buf1, c, U);
        add(buf1, b, U);
        add(buf1, e, U);
        add(buf1, e, U);
        uint8_t buf2[32] = {};
        uint8_t buf3[32] = {};
        for(int i=0; i<U; i++)
            buf2[i*2] = buf3[i*2] = buf1[i];
        naive(buf2, 2, 16);
        fft(buf3, 2, 16);
        assert(!memcmp(buf2, buf3, 2*U));
    }
    {
        memcpy(buf1, c, U);
        add(buf1, b, U);
        add(buf1, e, U);
        uint8_t buf2[32] = {};
        for(int i=0; i<U; i++)
            buf2[i*2] = buf1[i];
        naive(buf2, 2, 16);
        inaive(buf2, 2, 16);
        for(int i=0; i<U; i++)
            buf2[i] = buf2[2*i];
        assert(!memcmp(buf1, buf2, U));
    }
}

void test_karatsuba() {
    uint8_t buf1[U] = {};
    uint8_t buf2[U*2] = {};
    uint8_t buf3[U*2] = {};
    {
        memcpy(buf1, e, U);
        karatsuba(buf2, buf1, buf1, U);
        classical(buf3, buf1, buf1, U);
        assert(!memcmp(buf2, buf3, 2*U));
    }
    {
        uint8_t f[] = {0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
        uint8_t g[] = {0x6e, 0x34, 0x0b, 0x9c, 0xff, 0xb3, 0x7a, 0x98, 0x9c, 0xa5, 0x44, 0xe6, 0xbb, 0x78, 0x0a, 0x2c, 0x78, 0x90, 0x1d, 0x3f, 0xb3, 0x37, 0x38, 0x76, 0x85, 0x11, 0xa3, 0x06, 0x17, 0xaf, 0xa0, 0x1d};
        uint8_t buf2[sizeof(f)*2] = {};
        uint8_t buf3[sizeof(f)*2] = {};
        karatsuba(buf2, f, g, sizeof(f));
        classical(buf3, f, g, sizeof(f));
        assert(!memcmp(buf2, buf3, 2*sizeof(f)));
    }
#ifdef BENCH
    {
        uint8_t f[524288];
        uint8_t g[524288];
        for(int i=0; i<sizeof(f); i++) {
            f[i] = rand() & 0xFF;
            g[i] = rand() & 0xFF;
        }
        uint8_t buf2[sizeof(f)*2] = {};
        uint8_t buf3[sizeof(f)*2] = {};
        karatsuba(buf2, f, g, sizeof(f));
        classical64(buf3, f, g, sizeof(f));
        if(memcmp(buf2, buf3, 2*sizeof(f))) {
            print_hex(f, sizeof(f)); puts("f");
            print_hex(g, sizeof(f)); puts("g");
            puts("karatsuba result doesn't match");
            print_hex(buf2, 2*sizeof(f)); puts("karatsuba");
            print_hex(buf3, 2*sizeof(f)); puts("classical");
            assert(0);
        }
    }
#endif
    {
        for(int k=0; k<1000; k++) {
            uint8_t f[64];
            uint8_t g[64];
            for(int i=0; i<sizeof(f); i++) {
                f[i] = rand() & 0xFF;
                g[i] = rand() & 0xFF;
            }
            uint8_t buf2[sizeof(f)*2] = {};
            uint8_t buf3[sizeof(f)*2] = {};
            karatsuba(buf2, f, g, sizeof(f));
            classical64(buf3, f, g, sizeof(f));
            if(memcmp(buf2, buf3, 2*sizeof(f))) {
                print_hex(f, sizeof(f)); puts("f");
                print_hex(g, sizeof(f)); puts("g");
                puts("karatsuba result doesn't match");
                print_hex(buf2, 2*sizeof(f)); puts("karatsuba");
                print_hex(buf3, 2*sizeof(f)); puts("classical");
                assert(0);
            }
        }
    }
}
void test_mult() {
    uint8_t buf1[U] = {};
    uint8_t buf2[U*2] = {};
    uint8_t buf3[U*2] = {};
    {
        memcpy(buf1, e, U);
        mult(buf2, buf1, buf1, U);
        classical(buf3, buf1, buf1, U);
        assert(!memcmp(buf2, buf3, 2*U));
    }
    {
        uint8_t f[] = {0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
        uint8_t g[] = {0x6e, 0x34, 0x0b, 0x9c, 0xff, 0xb3, 0x7a, 0x98, 0x9c, 0xa5, 0x44, 0xe6, 0xbb, 0x78, 0x0a, 0x2c, 0x78, 0x90, 0x1d, 0x3f, 0xb3, 0x37, 0x38, 0x76, 0x85, 0x11, 0xa3, 0x06, 0x17, 0xaf, 0xa0, 0x1d};
        uint8_t buf2[sizeof(f)*2] = {};
        uint8_t buf3[sizeof(f)*2] = {};
        mult(buf2, f, g, sizeof(f));
        classical(buf3, f, g, sizeof(f));
        assert(!memcmp(buf2, buf3, 2*sizeof(f)));
    }
    {
        for(int k=0,b=0; k<10000; k++) {
            uint8_t f[32];
            uint8_t g[32];
            for(int i=0; i<sizeof(f); i++) {
                f[i] = rand() & 0xFF;
                g[i] = rand() & 0xFF;
            }
            uint8_t buf2[sizeof(f)*2] = {};
            uint8_t buf3[sizeof(f)*2] = {};
            if(!mult(buf2, f, g, sizeof(f))) {
                puts("fail");
                k--;
                b++;
                continue;
            }
            karatsuba(buf3, f, g, sizeof(f));
            printf("%d %d\n",k,b);
            if(memcmp(buf2, buf3, 2*sizeof(f))) {
                print_hex(f, sizeof(f)); puts("f");
                print_hex(g, sizeof(f)); puts("g");
                puts("mult result doesn't match");
                print_hex(buf2, 2*sizeof(f)); puts("ssa");
                print_hex(buf3, 2*sizeof(f)); puts("karatsuba");
                assert(0);
            }
        }
    }
}

int main(void) {
    srand(0);
    test_add();
    test_sub();
    test_shift();
    test_karatsuba();
    test_fft();
    test_mult();
}
