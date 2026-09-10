#include <limits.h>

void itoa(int value, char *buf) {
    char *start = buf;
    int negative = value < 0;

    if (negative) {
        *buf++ = '-';
        if (value == INT_MIN) {
            value = INT_MAX;
        } else {
            value = -value;
        }
    }

    char *digits = buf;
    if (value == 0) {
        *digits++ = '0';
    } else {
        while (value > 0) {
            *digits++ = (char)('0' + (value % 10));
            value /= 10;
        }
    }

    *digits = '\0';

    for (char *p = start, *q = digits - 1; p < q; p++, q--) {
        char tmp = *p;
        *p = *q;
        *q = tmp;
    }
}