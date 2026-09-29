#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int negative = 0;

    if (value == 0) {
        *p++ = '0';
    } else {
        if (value == INT_MIN) {
            *p++ = '-';
            unsigned int u = (unsigned int)(-(value + 1)) + 1u;
            while (u > 0) {
                *p++ = (char)('0' + (u % 10u));
                u /= 10u;
            }
        } else {
            if (value < 0) {
                negative = 1;
                value = -value;
            }
            while (value > 0) {
                *p++ = (char)('0' + (value % 10));
                value /= 10;
            }
            if (negative) {
                *p++ = '-';
            }
        }
    }

    while (p > buf) {
        char c = *--p;
        *p = *--p;
        *p = c;
    }

    *p = '\0';
}