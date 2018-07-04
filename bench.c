#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "common.h"
#include "mult.h"


int main(void) {
    struct timeval t1, t2;
    double sec, mic, passed;

    uint8_t *f = malloc(1<<27);
    uint8_t *g = malloc(1<<27);
    uint8_t buf2[2<<27] = {};
    uint8_t buf3[2<<27] = {};

    for(uint32_t bytes = 128; bytes <= 1<<27; bytes<<=1) {
    //for(uint32_t bytes = 134217728*2/8; bytes <= 1<<25; bytes<<=1) {
        double kpassed = 0, spassed = 0;
        const int N = 1;
        for(int k=0; k<N; k++){
            for(int i=0; i<bytes; i++) {
                f[i] = rand() & 0xFF;
                g[i] = rand() & 0xFF;
            }
            gettimeofday(&t1, NULL);
            if(!mult(buf2, f, g, bytes)) {
                puts("fail");
                bytes>>=1;
                continue;
            };
            gettimeofday(&t2, NULL);
            sec = (double)(t2.tv_sec - t1.tv_sec), mic = (double)(t2.tv_usec - t1.tv_usec), passed = sec + mic / 1000.0 / 1000.0;
            spassed += passed;
            continue;
            gettimeofday(&t1, NULL);
            karatsuba(buf3, f, g, bytes);
            gettimeofday(&t2, NULL);
            sec = (double)(t2.tv_sec - t1.tv_sec), mic = (double)(t2.tv_usec - t1.tv_usec), passed = sec + mic / 1000.0 / 1000.0;
            kpassed += passed;
            if(memcmp(buf2, buf3, 2*bytes)) {
                print_hex(f, bytes); puts("f");
                print_hex(g, bytes); puts("g");
                puts("mult result doesn't match");
                print_hex(buf2, 2*bytes); puts("ssa");
                print_hex(buf3, 2*bytes); puts("karatsuba");
                fflush(stdout);
                assert(0);
            }
        }
        printf("%8d : %.16f %.16f\n", 8*bytes, kpassed/N, spassed/N);
    }
    return 0;
}
