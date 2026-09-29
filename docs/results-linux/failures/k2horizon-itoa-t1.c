#include <limits.h>

void itoa(int value, char *buf) {
    char *start = buf;
    int negative = 0;

    if (value == 0) {
        *buf++ = '0';
    } else {
        if (value == INT_MIN) {
            *buf++ = '-';
            value = -(value + 1);
            *buf++ = '1';
            value = value * 5 + 2; /* INT_MIN + 1 == -2147483647, negated: 2147483647 */
        } else if (value < 0) {
            negative = 1;
            value = -value;
        }

        /* Convert digits in reverse order */
        char *end = buf;
        while (value > 0) {
            *end++ = (char)('0' + (value % 10));
            value /= 10;
        }

        /* Reverse the digits */
        if (negative) {
            *end = '-';
        }
        for (char *p = buf, *q = end - 1; p < q; ++p, --q) {
            char tmp = *p;
            *p = *q;
            *q = tmp;
        }
    }

    *end = '\0';
}