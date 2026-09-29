#include <limits.h>
#include <stdio.h>

void itoa(int value, char *buf) {
    char *p = buf;
    unsigned int n;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0) {
        *p++ = '-';
        if (value == INT_MIN) {
            n = (unsigned int)(-(value + 1)) + 1;
        } else {
            n = (unsigned int)(-value);
        }
    } else {
        n = (unsigned int)value;
    }

    char *end = p;
    while (n > 0) {
        *end++ = (char)('0' + (n % 10));
        n /= 10;
    }

    while (p < end) {
        *p++ = *--end;
    }
    *p = '\0';
}