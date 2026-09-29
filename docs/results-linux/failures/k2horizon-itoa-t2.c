#include <limits.h>

void itoa(int value, char *buf) {
    char *start = buf;

    if (value == 0) {
        *buf++ = '0';
    } else {
        int negative = 0;

        /* Handle INT_MIN safely using long long */
        long long v = value;

        if (v < 0) {
            negative = 1;
            v = -(v);
        }

        /* Build digits in reverse order */
        char *digits = buf;
        while (v > 0) {
            *digits++ = (char)('0' + (v % 10));
            v /= 10;
        }

        /* Reverse the digits */
        char *end = digits - 1;
        while (end > buf) {
            char tmp = *buf;
            *buf++ = *end;
            *end-- = tmp;
        }

        if (negative) {
            *buf++ = '-';
        }
    }

    *buf = '\0';
}