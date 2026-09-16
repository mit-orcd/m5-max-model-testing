#include <limits.h>

void itoa(int value, char *buf) {
    char *ptr = buf;
    int negative = 0;

    if (value == INT_MIN) {
        *ptr++ = '-';
        unsigned int u = (unsigned int)(-(value + 1)) + 1u;
        char *start = ptr;
        do {
            *ptr++ = (char)('0' + (u % 10u));
            u /= 10u;
        } while (u != 0u);
        while (ptr > start + 1)
            *--ptr = *(ptr + 1);
        *ptr = '\0';
        return;
    }

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    char *start = ptr;
    while (value > 0) {
        *ptr++ = (char)('0' + (value % 10));
        value /= 10;
    }
    if (negative)
        *ptr++ = '-';

    while (ptr > start + (negative ? 1 : 0))
        *--ptr = *(ptr + 1);

    *ptr = '\0';
}