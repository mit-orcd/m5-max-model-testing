#include <limits.h>

void itoa(int value, char *buf) {
    char tmp[12]; // Enough for 32-bit int including sign and null terminator
    int i = 0;
    unsigned int u;

    if (value < 0) {
        u = -(unsigned int)value;
    } else {
        u = (unsigned int)value;
    }

    if (u == 0) {
        buf[i++] = '0';
    } else {
        while (u > 0) {
            tmp[i++] = (char)('0' + (u % 10));
            u /= 10;
        }
    }

    if (value < 0) {
        tmp[i++] = '-';
    }

    // Reverse tmp into buf
    int j;
    for (j = 0; j < i; j++) {
        buf[j] = tmp[i - 1 - j];
    }
    buf[i] = '\0';
}