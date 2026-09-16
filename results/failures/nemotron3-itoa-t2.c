#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    long v = value;
    int negative = 0;

    if (v < 0) {
        negative = 1;
        v = -v;
    }

    if (v == 0) {
        *p++ = '0';
    } else {
        char temp[32];
        int i = 0;
        while (v > 0) {
            temp[i++] = '0' + (v % 10);
            v /= 10;
        }
        while (i > 0) {
            *p++ = temp[--i];
        }
    }

    if (negative) {
        *p++ = '-';
    }

    *p = '\0';
}