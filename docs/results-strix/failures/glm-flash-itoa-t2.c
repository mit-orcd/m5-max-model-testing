#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    unsigned int uvalue;

    if (value == INT_MIN) {
        uvalue = (unsigned int)(value + 1);
        *p++ = '-';
    } else {
        uvalue = (unsigned int)value;
        if (value < 0) {
            uvalue = (unsigned int)(-value);
            *p++ = '-';
        }
    }

    if (uvalue == 0) {
        *p++ = '0';
    } else {
        char *tmp = p;
        while (uvalue > 0) {
            *tmp++ = (uvalue % 10) + '0';
            uvalue /= 10;
        }
        *p = '\0';
        tmp--;

        while (p < tmp) {
            char c = *p;
            *p++ = *tmp;
            *tmp-- = c;
        }
    }
}