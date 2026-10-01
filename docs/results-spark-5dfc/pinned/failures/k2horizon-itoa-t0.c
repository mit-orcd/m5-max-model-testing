#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int negative = value < 0;

    if (negative) {
        *p++ = '-';
        if (value == INT_MIN) {
            value = INT_MAX;
        } else {
            value = -value;
        }
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        char *start = p;
        while (value > 0) {
            *p++ = (char)('0' + (value % 10));
            value /= 10;
        }
        while (p > start) {
            *--p = *p;
        }
    }

    *p = '\0';
}