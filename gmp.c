#include <stdio.h>
#include <sys/time.h>
#include <gmp.h>

int main(void) {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    mpz_t a, b, c;
    mpz_init(a);
    mpz_init(b);
    mpz_init(c);
    mpz_urandomb(a, state, 134217728);
    mpz_urandomb(b, state, 134217728);

    struct timeval t1, t2;
    double sec, mic, passed;

    gettimeofday(&t1, NULL);
    mpz_mul(c, a, b);
    gettimeofday(&t2, NULL);
    //mpz_out_str(stdout, 16, c);
    sec = (double)(t2.tv_sec - t1.tv_sec), mic = (double)(t2.tv_usec - t1.tv_usec), passed = sec + mic / 1000.0 / 1000.0;
    printf("%.16f\n", passed);
}
