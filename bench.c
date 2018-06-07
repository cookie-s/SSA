#include "common.h"
#include "fft.c"

#include <sys/time.h>

int main(void) {
    struct timeval t1, t2;
    double sec, mic, passed;

    for(int k=0; k<1; k++){
        uint8_t f[1<<(14-3)];
        uint8_t g[1<<(14-3)];
        for(int i=0; i<sizeof(f); i++) {
            f[i] = rand() & 0xFF;
            g[i] = rand() & 0xFF;
        }
        for(int i=0; i<sizeof(f); i++) {
            f[i] = 0;
            g[i] = 0;
        }
        f[0] = g[0] = 1;
        uint8_t buf2[sizeof(f)*2] = {};
        uint8_t buf3[sizeof(f)*2] = {};
        printf("%d bits\n", sizeof(f)*8);
        gettimeofday(&t1, NULL);
        mult(buf2, f, g, sizeof(f));
        gettimeofday(&t2, NULL);
        sec = (double)(t2.tv_sec - t1.tv_sec), mic = (double)(t2.tv_usec - t1.tv_usec), passed = sec + mic / 1000.0 / 1000.0;
        printf("ssa done: %.16f\n", passed);
        gettimeofday(&t1, NULL);
        karatsuba(buf3, f, g, sizeof(f));
        gettimeofday(&t2, NULL);
        sec = (double)(t2.tv_sec - t1.tv_sec), mic = (double)(t2.tv_usec - t1.tv_usec), passed = sec + mic / 1000.0 / 1000.0;
        printf("karatsuba done: %.16f\n", passed);
        if(memcmp(buf2, buf3, 2*sizeof(f))) {
            print_hex(f, sizeof(f)); puts("f");
            print_hex(g, sizeof(f)); puts("g");
            puts("mult result doesn't match");
            print_hex(buf2, 2*sizeof(f)); puts("mult");
            print_hex(buf3, 2*sizeof(f)); puts("classical");
            assert(0);
        }
    }
    return 0;
}
