#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "fft.h"
#include "mult.h"
#include "io.h"

int main(void) {
    uint8_t f[16];
    uint8_t g[16];
    printf("%ld\n", sizeof(f));

    for(int k=0; ; k++) {
        printf("f>\n");
        if(!scan_hex(f, sizeof(f))) return 0;
        printf("g>\n");
        if(!scan_hex(g, sizeof(f))) return 0;
        uint8_t buf2[sizeof(f)*2] = {};
        uint8_t buf3[sizeof(f)*2] = {};
        printf("success: %d\n", mult(buf2, f, g, sizeof(f)));
        karatsuba(buf3, f, g, sizeof(f));
        if(memcmp(buf2, buf3, 2*sizeof(f)))
            puts("!!! not match !!!");
        print_hex(f, sizeof(f)); puts("f");
        print_hex(g, sizeof(f)); puts("g");
        print_hex(buf2, 2*sizeof(f)); puts("ssa");
        print_hex(buf3, 2*sizeof(f)); puts("karatsuba");
    }
}
