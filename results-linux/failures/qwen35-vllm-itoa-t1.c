#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    char *q = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        /* INT_MIN cannot be negated directly as -INT_MIN is undefined behavior */
        /* We handle digits manually */
        while (1) {
            int digit = value % -10;
            value /= -10;
            *p++ = '0' - digit;
            if (value == 0) break;
        }
        *p = '\0';
        /* Reverse the string in place */
        while (q < p - 1) {
            char tmp = *q;
            *q++ = *--p;
            *p = tmp;
        }
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        while (value > 0) {
            *p++ = '0' + (value % 10);
            value /= 10;
        }
    }

    if (is_negative) {
        *p++ = '-';
    }

    *p = '\0';

    /* Reverse the string in place */
    while (q < p - 1) {
        char tmp = *q;
        *q++ = *--p;
        *p = tmp;
    }
}