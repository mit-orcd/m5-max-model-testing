#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int is_negative = 0;
    unsigned int uvalue;

    if (value < 0) {
        is_negative = 1;
        /* Handle INT_MIN safely by converting to unsigned */
        uvalue = (unsigned int)(-(value + 1)) + 1u;
    } else {
        uvalue = (unsigned int)value;
    }

    if (uvalue == 0) {
        *p++ = '0';
    } else {
        char tmp[10];
        int i = 0;
        while (uvalue > 0) {
            tmp[i++] = (char)('0' + (uvalue % 10));
            uvalue /= 10;
        }
        while (i > 0) {
            *p++ = tmp[--i];
        }
    }

    if (is_negative) {
        *p++ = '-';
    }

    *p = '\0';
}