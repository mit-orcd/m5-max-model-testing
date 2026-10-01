#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int negative = 0;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value == INT_MIN) {
        negative = 1;
        value = INT_MAX;
    } else if (value < 0) {
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

    *p = '\0';

    /* Reverse the digits */
    char *start = buf;
    char *end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}