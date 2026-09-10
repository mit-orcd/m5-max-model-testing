#include <limits.h>

void itoa(int value, char *buf) {
    char *start = buf;
    int negative = 0;

    if (value == 0) {
        *buf++ = '0';
        *buf = '\0';
        return;
    }

    if (value == INT_MIN) {
        negative = 1;
        value = -(value + 1);
        *buf++ = '7';
    } else if (value < 0) {
        negative = 1;
        value = -value;
    }

    while (value > 0) {
        *buf++ = (char)('0' + (value % 10));
        value /= 10;
    }

    if (negative) {
        *buf++ = '-';
    }

    *buf = '\0';

    for (char *p = start, *q = buf - 1; p < q; p++, q--) {
        char tmp = *p;
        *p = *q;
        *q = tmp;
    }
}