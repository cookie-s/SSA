#include <stdio.h>
#include <ctype.h>

#include "io.h"

char scan_hex(uint8_t *d, uint32_t n) {
    uint32_t i=0;
    while(i<n) {
        uint8_t b = 0;

        while(1) {
            int c = getchar();
            if(c == EOF) return 0;
            if(isdigit(c)) {
                b |= c - '0';
                break;
            }
            if('a' <= c && c <= 'f') {
                b |= c - 'a' + 10;
                break;
            }
            if('A' <= c && c <= 'F') {
                b |= c - 'A' + 10;
                break;
            }
        }

        b <<= 4;

        while(1) {
            int c = getchar();
            if(c == EOF) return 0;
            if(isdigit(c)) {
                b |= c - '0';
                break;
            }
            if('a' <= c && c <= 'f') {
                b |= c - 'a' + 10;
                break;
            }
            if('A' <= c && c <= 'F') {
                b |= c - 'A' + 10;
                break;
            }
        }

        d[i++] = b;
    }

    return 1;
}
