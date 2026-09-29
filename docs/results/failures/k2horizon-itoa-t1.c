#include <limits.h>

void itoa(int value, char *buf) {
    unsigned int uvalue;
    char *start = buf;
    char *p;

    if (value == 0) {
        *buf++ = '0';
        *buf = '\0';
        return;
    }

    if (value == INT_MIN) {
        uvalue = (unsigned int)INT_MAX + 1u;
    } else {
        uvalue = (value < 0) ? (unsigned int)(-(value + 1)) + 1u : (unsigned int)value;
    }

    if (value < 0) {
        *buf++ = '-';
    }

    p = buf + sizeof(unsigned int) - 1;
    *p = '\0';

    while (uvalue != 0) {
        *--p = (char)('0' + (uvalue % 10u));
        uvalue /= 10u;
    }

    while (p < buf + sizeof(unsigned int) - 1) {
        *buf++ = *p++;
    }
    *buf = '\0';
}