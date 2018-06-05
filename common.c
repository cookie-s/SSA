#include "common.h"

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

